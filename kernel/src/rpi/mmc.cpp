#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "timer.h"
#include "mmc.h"
#include "device.h"

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID         400000
#define SD_CLOCK_NORMAL     25000000

#define TIMEOUT_WAIT(stmt, timeout) { size_t _time = timeout / 1000; while (_time && !(stmt)) { wait_us(100); _time--; } }

struct sd_scr
{
    uint32_t    scr[2];
    uint32_t    sd_bus_widths;
    int         sd_version;
};

static uint32_t card_supports_sdhc;
static uint32_t card_supports_18v;
static uint32_t card_ocr;
static uint32_t card_rca;
static uint32_t last_interrupt;
static uint32_t last_error;
static struct sd_scr scr;
static bool failed_voltage_switch = false;
static uint32_t last_cmd;
static uint32_t last_cmd_success;
static uint32_t last_r0;
static uint32_t last_r1;
static uint32_t last_r2;
static uint32_t last_r3;
static void *buf;
static int blocks_to_transfer;
static size_t block_size;
static int card_removal;

struct emmc_regs {
  uint32_t arg2;
  uint32_t blksizecnt;
  uint32_t arg1;
  uint32_t cmdtm;
  uint32_t resp0;
  uint32_t resp1;
  uint32_t resp2;
  uint32_t resp3;
  uint32_t data;
  uint32_t status;
  uint32_t control0;
  uint32_t control1;
  uint32_t interrupt;
  uint32_t interrupt_mask;
  uint32_t interrupt_enable;
  uint32_t control2;
  uint32_t capabilities0;
  uint32_t capabilities1;
  uint32_t pad[2];
  uint32_t force_irpt;
  uint32_t pad2[7];
  uint32_t boot_timeout;
  uint32_t dbg_sel;
  uint32_t pad3[2];
  uint32_t exrdfifo_cfg;
  uint32_t exrdfifo_en;
  uint32_t tune_step;
  uint32_t tune_steps_std;
  uint32_t tune_steps_ddr;
  uint32_t pad4[23];
  uint32_t spi_int_spt;
  uint32_t pad5[2];
  uint32_t slotisr_ver;
};
static volatile emmc_regs* regs = (emmc_regs*)0x20300000;

#define SD_CMD_INDEX(a)   ((a) << 24)
#define SD_CMD_TYPE_NORMAL  0x0
#define SD_CMD_TYPE_SUSPEND (1 << 22)
#define SD_CMD_TYPE_RESUME  (2 << 22)
#define SD_CMD_TYPE_ABORT (3 << 22)
#define SD_CMD_TYPE_MASK    (3 << 22)
#define SD_CMD_ISDATA   (1 << 21)
#define SD_CMD_IXCHK_EN   (1 << 20)
#define SD_CMD_CRCCHK_EN  (1 << 19)
#define SD_CMD_RSPNS_TYPE_NONE  0     // For no response
#define SD_CMD_RSPNS_TYPE_136 (1 << 16)   // For response R2 (with CRC), R3,4 (no CRC)
#define SD_CMD_RSPNS_TYPE_48  (2 << 16)   // For responses R1, R5, R6, R7 (with CRC)
#define SD_CMD_RSPNS_TYPE_48B (3 << 16)   // For responses R1b, R5b (with CRC)
#define SD_CMD_RSPNS_TYPE_MASK  (3 << 16)
#define SD_CMD_MULTI_BLOCK  (1 << 5)
#define SD_CMD_DAT_DIR_HC 0
#define SD_CMD_DAT_DIR_CH (1 << 4)
#define SD_CMD_AUTO_CMD_EN_NONE 0
#define SD_CMD_AUTO_CMD_EN_CMD12  (1 << 2)
#define SD_CMD_AUTO_CMD_EN_CMD23  (2 << 2)
#define SD_CMD_BLKCNT_EN    (1 << 1)
#define SD_CMD_DMA          1

#define SD_ERR_CMD_TIMEOUT  0
#define SD_ERR_CMD_CRC    1
#define SD_ERR_CMD_END_BIT  2
#define SD_ERR_CMD_INDEX  3
#define SD_ERR_DATA_TIMEOUT 4
#define SD_ERR_DATA_CRC   5
#define SD_ERR_DATA_END_BIT 6
#define SD_ERR_CURRENT_LIMIT  7
#define SD_ERR_AUTO_CMD12 8
#define SD_ERR_ADMA   9
#define SD_ERR_TUNING   10
#define SD_ERR_RSVD   11

#define SD_ERR_MASK_CMD_TIMEOUT   (1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_CMD_CRC   (1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_CMD_END_BIT   (1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CMD_INDEX   (1 << (16 + SD_ERR_CMD_INDEX))
#define SD_ERR_MASK_DATA_TIMEOUT  (1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_DATA_CRC    (1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_DATA_END_BIT  (1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CURRENT_LIMIT (1 << (16 + SD_ERR_CMD_CURRENT_LIMIT))
#define SD_ERR_MASK_AUTO_CMD12    (1 << (16 + SD_ERR_CMD_AUTO_CMD12))
#define SD_ERR_MASK_ADMA    (1 << (16 + SD_ERR_CMD_ADMA))
#define SD_ERR_MASK_TUNING    (1 << (16 + SD_ERR_CMD_TUNING))

#define SD_COMMAND_COMPLETE     1
#define SD_TRANSFER_COMPLETE    (1 << 1)
#define SD_BLOCK_GAP_EVENT      (1 << 2)
#define SD_DMA_INTERRUPT        (1 << 3)
#define SD_BUFFER_WRITE_READY   (1 << 4)
#define SD_BUFFER_READ_READY    (1 << 5)
#define SD_CARD_INSERTION       (1 << 6)
#define SD_CARD_REMOVAL         (1 << 7)
#define SD_CARD_INTERRUPT       (1 << 8)

#define SD_RESP_NONE        SD_CMD_RSPNS_TYPE_NONE
#define SD_RESP_R1          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R1b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R2          (SD_CMD_RSPNS_TYPE_136 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R3          SD_CMD_RSPNS_TYPE_48
#define SD_RESP_R4          SD_CMD_RSPNS_TYPE_136
#define SD_RESP_R5          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R5b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R6          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R7          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)

#define SD_DATA_READ        (SD_CMD_ISDATA | SD_CMD_DAT_DIR_CH)
#define SD_DATA_WRITE       (SD_CMD_ISDATA | SD_CMD_DAT_DIR_HC)

#define SD_CMD_RESERVED(a)  0xffffffff

#define SUCCESS             (last_cmd_success)
#define FAIL                (last_cmd_success == 0)
#define TIMEOUT             (FAIL && (last_error == 0))
#define CMD_TIMEOUT         (FAIL && (last_error & (1 << 16)))
#define CMD_CRC             (FAIL && (last_error & (1 << 17)))
#define CMD_END_BIT         (FAIL && (last_error & (1 << 18)))
#define CMD_INDEX           (FAIL && (last_error & (1 << 19)))
#define DATA_TIMEOUT        (FAIL && (last_error & (1 << 20)))
#define DATA_CRC            (FAIL && (last_error & (1 << 21)))
#define DATA_END_BIT        (FAIL && (last_error & (1 << 22)))
#define CURRENT_LIMIT       (FAIL && (last_error & (1 << 23)))
#define ACMD12_ERROR        (FAIL && (last_error & (1 << 24)))
#define ADMA_ERROR          (FAIL && (last_error & (1 << 25)))
#define TUNING_ERROR        (FAIL && (last_error & (1 << 26)))

#define SD_VER_UNKNOWN      0
#define SD_VER_1            1
#define SD_VER_1_1          2
#define SD_VER_2            3
#define SD_VER_3            4
#define SD_VER_4            5

static uint32_t sd_commands[] = {
    SD_CMD_INDEX(0),
    SD_CMD_RESERVED(1),
    SD_CMD_INDEX(2) | SD_RESP_R2,
    SD_CMD_INDEX(3) | SD_RESP_R6,
    SD_CMD_INDEX(4),
    SD_CMD_INDEX(5) | SD_RESP_R4,
    SD_CMD_INDEX(6) | SD_RESP_R1,
    SD_CMD_INDEX(7) | SD_RESP_R1b,
    SD_CMD_INDEX(8) | SD_RESP_R7,
    SD_CMD_INDEX(9) | SD_RESP_R2,
    SD_CMD_INDEX(10) | SD_RESP_R2,
    SD_CMD_INDEX(11) | SD_RESP_R1,
    SD_CMD_INDEX(12) | SD_RESP_R1b | SD_CMD_TYPE_ABORT,
    SD_CMD_INDEX(13) | SD_RESP_R1,
    SD_CMD_RESERVED(14),
    SD_CMD_INDEX(15),
    SD_CMD_INDEX(16) | SD_RESP_R1,
    SD_CMD_INDEX(17) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(18) | SD_RESP_R1 | SD_DATA_READ | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN,
    SD_CMD_INDEX(19) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(20) | SD_RESP_R1b,
    SD_CMD_RESERVED(21),
    SD_CMD_RESERVED(22),
    SD_CMD_INDEX(23) | SD_RESP_R1,
    SD_CMD_INDEX(24) | SD_RESP_R1 | SD_DATA_WRITE,
    SD_CMD_INDEX(25) | SD_RESP_R1 | SD_DATA_WRITE | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN,
    SD_CMD_RESERVED(26),
    SD_CMD_INDEX(27) | SD_RESP_R1 | SD_DATA_WRITE,
    SD_CMD_INDEX(28) | SD_RESP_R1b,
    SD_CMD_INDEX(29) | SD_RESP_R1b,
    SD_CMD_INDEX(30) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_RESERVED(31),
    SD_CMD_INDEX(32) | SD_RESP_R1,
    SD_CMD_INDEX(33) | SD_RESP_R1,
    SD_CMD_RESERVED(34),
    SD_CMD_RESERVED(35),
    SD_CMD_RESERVED(36),
    SD_CMD_RESERVED(37),
    SD_CMD_INDEX(38) | SD_RESP_R1b,
    SD_CMD_RESERVED(39),
    SD_CMD_RESERVED(40),
    SD_CMD_RESERVED(41),
    SD_CMD_RESERVED(42) | SD_RESP_R1,
    SD_CMD_RESERVED(43),
    SD_CMD_RESERVED(44),
    SD_CMD_RESERVED(45),
    SD_CMD_RESERVED(46),
    SD_CMD_RESERVED(47),
    SD_CMD_RESERVED(48),
    SD_CMD_RESERVED(49),
    SD_CMD_RESERVED(50),
    SD_CMD_RESERVED(51),
    SD_CMD_RESERVED(52),
    SD_CMD_RESERVED(53),
    SD_CMD_RESERVED(54),
    SD_CMD_INDEX(55) | SD_RESP_R1,
    SD_CMD_INDEX(56) | SD_RESP_R1 | SD_CMD_ISDATA,
    SD_CMD_RESERVED(57),
    SD_CMD_RESERVED(58),
    SD_CMD_RESERVED(59),
    SD_CMD_RESERVED(60),
    SD_CMD_RESERVED(61),
    SD_CMD_RESERVED(62),
    SD_CMD_RESERVED(63)
};

static uint32_t sd_acommands[] = {
    SD_CMD_RESERVED(0),
    SD_CMD_RESERVED(1),
    SD_CMD_RESERVED(2),
    SD_CMD_RESERVED(3),
    SD_CMD_RESERVED(4),
    SD_CMD_RESERVED(5),
    SD_CMD_INDEX(6) | SD_RESP_R1,
    SD_CMD_RESERVED(7),
    SD_CMD_RESERVED(8),
    SD_CMD_RESERVED(9),
    SD_CMD_RESERVED(10),
    SD_CMD_RESERVED(11),
    SD_CMD_RESERVED(12),
    SD_CMD_INDEX(13) | SD_RESP_R1,
    SD_CMD_RESERVED(14),
    SD_CMD_RESERVED(15),
    SD_CMD_RESERVED(16),
    SD_CMD_RESERVED(17),
    SD_CMD_RESERVED(18),
    SD_CMD_RESERVED(19),
    SD_CMD_RESERVED(20),
    SD_CMD_RESERVED(21),
    SD_CMD_INDEX(22) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(23) | SD_RESP_R1,
    SD_CMD_RESERVED(24),
    SD_CMD_RESERVED(25),
    SD_CMD_RESERVED(26),
    SD_CMD_RESERVED(27),
    SD_CMD_RESERVED(28),
    SD_CMD_RESERVED(29),
    SD_CMD_RESERVED(30),
    SD_CMD_RESERVED(31),
    SD_CMD_RESERVED(32),
    SD_CMD_RESERVED(33),
    SD_CMD_RESERVED(34),
    SD_CMD_RESERVED(35),
    SD_CMD_RESERVED(36),
    SD_CMD_RESERVED(37),
    SD_CMD_RESERVED(38),
    SD_CMD_RESERVED(39),
    SD_CMD_RESERVED(40),
    SD_CMD_INDEX(41) | SD_RESP_R3,
    SD_CMD_INDEX(42) | SD_RESP_R1,
    SD_CMD_RESERVED(43),
    SD_CMD_RESERVED(44),
    SD_CMD_RESERVED(45),
    SD_CMD_RESERVED(46),
    SD_CMD_RESERVED(47),
    SD_CMD_RESERVED(48),
    SD_CMD_RESERVED(49),
    SD_CMD_RESERVED(50),
    SD_CMD_INDEX(51) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_RESERVED(52),
    SD_CMD_RESERVED(53),
    SD_CMD_RESERVED(54),
    SD_CMD_RESERVED(55),
    SD_CMD_RESERVED(56),
    SD_CMD_RESERVED(57),
    SD_CMD_RESERVED(58),
    SD_CMD_RESERVED(59),
    SD_CMD_RESERVED(60),
    SD_CMD_RESERVED(61),
    SD_CMD_RESERVED(62),
    SD_CMD_RESERVED(63)
};

// The actual command indices
#define GO_IDLE_STATE           0
#define ALL_SEND_CID            2
#define SEND_RELATIVE_ADDR      3
#define SET_DSR                 4
#define IO_SET_OP_COND          5
#define SWITCH_FUNC             6
#define SELECT_CARD             7
#define DESELECT_CARD           7
#define SELECT_DESELECT_CARD    7
#define SEND_IF_COND            8
#define SEND_CSD                9
#define SEND_CID                10
#define VOLTAGE_SWITCH          11
#define STOP_TRANSMISSION       12
#define SEND_STATUS             13
#define GO_INACTIVE_STATE       15
#define SET_BLOCKLEN            16
#define READ_SINGLE_BLOCK       17
#define READ_MULTIPLE_BLOCK     18
#define SEND_TUNING_BLOCK       19
#define SPEED_CLASS_CONTROL     20
#define SET_BLOCK_COUNT         23
#define WRITE_BLOCK             24
#define WRITE_MULTIPLE_BLOCK    25
#define PROGRAM_CSD             27
#define SET_WRITE_PROT          28
#define CLR_WRITE_PROT          29
#define SEND_WRITE_PROT         30
#define ERASE_WR_BLK_START      32
#define ERASE_WR_BLK_END        33
#define ERASE                   38
#define LOCK_UNLOCK             42
#define APP_CMD                 55
#define GEN_CMD                 56

#define IS_APP_CMD              0x80000000
#define ACMD(a)                 (a | IS_APP_CMD)
#define SET_BUS_WIDTH           (6 | IS_APP_CMD)
#define SD_STATUS               (13 | IS_APP_CMD)
#define SEND_NUM_WR_BLOCKS      (22 | IS_APP_CMD)
#define SET_WR_BLK_ERASE_COUNT  (23 | IS_APP_CMD)
#define SD_SEND_OP_COND         (41 | IS_APP_CMD)
#define SET_CLR_CARD_DETECT     (42 | IS_APP_CMD)
#define SEND_SCR                (51 | IS_APP_CMD)

#define SD_RESET_CMD            (1 << 25)
#define SD_RESET_DAT            (1 << 26)
#define SD_RESET_ALL            (1 << 24)

#define SD_GET_CLOCK_DIVIDER_FAIL 0xffffffff

#include "property.h"

static void sd_power_off()
{
    regs->control0 = (regs->control0 & ~(1 << 8));
}

static uint32_t sd_get_base_clock_hz()
{
    int clockid = 1;
    uint32_t base_clock = ((uint32_t*)property_read(PropertyMmcBaseClock, 8, &clockid, 4))[1];

    //printf("[EMMC] base clock rate is %d Hz\n", base_clock);
    return base_clock;
}

static int bcm_2708_power_cycle()
{
    struct powerState {
        int deviceId;
        int powerState;
    } state;
    state.deviceId = 0;
    state.powerState = 2;
    powerState *p = (powerState*)property_read(PropertyPowerState, 8, &state, 8);

    if (p->deviceId != 0 || (p->powerState & 3) != 0)
    {
        //printf("[EMMC] device did not power off successfully.\n");
    }

    wait_us(5000);

    state.powerState = 3;
    powerState *p2 = (powerState*)property_read(PropertyPowerState, 8, &state, 8);
    if (p2->deviceId != 0 || (p2->powerState & 3) != 1)
    {
        //printf("[EMMC] device did not power on successfully.\n");
    }

    //printf("[EMMC] power cycled\n");
    return 0;
}

// Set the clock dividers to generate a target value
static uint32_t sd_get_clock_divider(uint32_t base_clock, uint32_t target_rate)
{
    // TODO: implement use of preset value registers

    uint32_t targetted_divisor = 0;
    if(target_rate > base_clock)
        targetted_divisor = 1;
    else
    {
        targetted_divisor = base_clock / target_rate;
        uint32_t mod = base_clock % target_rate;
        if(mod)
            targetted_divisor--;
    }

    // Decide on the clock mode to use

    // Currently only 10-bit divided clock mode is supported

    // HCI version 3 or greater supports 10-bit divided clock mode
    // This requires a power-of-two divider

    // Find the first bit set
    int divisor = -1;
    for(int first_bit = 31; first_bit >= 0; first_bit--)
    {
        uint32_t bit_test = (1 << first_bit);
        if(targetted_divisor & bit_test)
        {
            divisor = first_bit;
            targetted_divisor &= ~bit_test;
            if(targetted_divisor)
            {
                // The divisor is not a power-of-two, increase it
                divisor++;
            }
            break;
        }
    }

    if(divisor == -1)
        divisor = 31;
    if(divisor >= 32)
        divisor = 31;

    if(divisor != 0)
        divisor = (1 << (divisor - 1));

    if(divisor >= 0x400)
        divisor = 0x3ff;

    uint32_t freq_select = divisor & 0xff;
    uint32_t upper_bits = (divisor >> 8) & 0x3;
    uint32_t ret = (freq_select << 8) | (upper_bits << 6) | (0 << 5);

//    int denominator = 1;
//    if(divisor != 0)
//        denominator = divisor * 2;
//    int actual_clock = base_clock / denominator;
    //printf("[EMMC] base_clock: %d, target_rate: %d, divisor: %08X, "
//           "actual_clock: %d, ret: %08X\n", base_clock, target_rate,
//           divisor, actual_clock, ret);

    return ret;
}

// Switch the clock rate whilst running
static int sd_switch_clock_rate(uint32_t base_clock, uint32_t target_rate)
{
    // Decide on an appropriate divider
    uint32_t divider = sd_get_clock_divider(base_clock, target_rate);
    if(divider == SD_GET_CLOCK_DIVIDER_FAIL)
    {
        //printf("[EMMC] couldn't get a valid divider for target rate %d Hz\n",
//               target_rate);
        return -1;
    }

    // Wait for the command inhibit (CMD and DAT) bits to clear
    while(regs->status & 3)
        wait_us(1000);

    // Set the SD clock off
    regs->control1 = regs->control1 & ~(1 << 2);
    wait_us(2000);

    // Write the new divider
    regs->control1 = (regs->control1 & 0xFFFF001FU) | divider;
    wait_us(2000);

    // Enable the SD clock
    regs->control1 = regs->control1 | 0x4;
    wait_us(2000);

    //printf("[EMMC] successfully set clock rate to %d Hz\n", target_rate);
    return 0;
}

// Reset the CMD line
static int sd_reset_cmd()
{
    regs->control1 = regs->control1 | SD_RESET_CMD;
    TIMEOUT_WAIT((regs->control1 & SD_RESET_CMD) == 0, 1000000);
    if((regs->control1 & SD_RESET_CMD) != 0)
    {
        //printf("[EMMC] CMD line did not reset properly\n");
        return -1;
    }
    return 0;
}

// Reset the CMD line
static int sd_reset_dat()
{
    regs->control1 = regs->control1 | SD_RESET_DAT;
    TIMEOUT_WAIT((regs->control1 & SD_RESET_DAT) == 0, 1000000);
    if((regs->control1 & SD_RESET_DAT) != 0)
    {
        //printf("[EMMC] DAT line did not reset properly\n");
        return -1;
    }
    return 0;
}

static void sd_issue_command_int(uint32_t cmd_reg, uint32_t argument, size_t timeout)
{
    last_cmd_success = 0;

    // This is as per HCSS 3.7.1.1/3.7.2.2

    // Check Command Inhibit
    while(regs->status & 0x1)
        wait_us(1000);

    // Is the command with busy?
    if((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B &&
       (cmd_reg & SD_CMD_TYPE_MASK) != SD_CMD_TYPE_ABORT)
    {
        // Not an abort command

        // Wait for the data line to be free
        while(regs->status & 0x2)
            wait_us(1000);
    }

    // Set block size and block count
    // For now, block size = 512 bytes, block count = 1,
    if(blocks_to_transfer > 0xffff)
    {
        //printf("[EMMC] blocks_to_transfer too great (%d)\n",
//               blocks_to_transfer);
        last_cmd_success = 0;
        return;
    }
    uint32_t blksizecnt = block_size | (blocks_to_transfer << 16);
    regs->blksizecnt = blksizecnt;

    // Set argument 1 reg
    regs->arg1 = argument;

    // Set command reg
    regs->cmdtm = cmd_reg;

    wait_us(2000);

    // Wait for command complete interrupt
    TIMEOUT_WAIT(regs->interrupt & 0x8001, timeout);
    uint32_t irpts = regs->interrupt;

    // Clear command complete status
    regs->interrupt = 0xFFFF0001;

    // Test for errors
    if((irpts & 0xffff0001) != 0x1)
    {
        //printf("[EMMC] error occured whilst waiting for command complete interrupt\n");
        last_error = irpts & 0xffff0000;
        last_interrupt = irpts;
        return;
    }

    wait_us(2000);

    // Get response data
    switch(cmd_reg & SD_CMD_RSPNS_TYPE_MASK)
    {
    case SD_CMD_RSPNS_TYPE_48:
    case SD_CMD_RSPNS_TYPE_48B:
        last_r0 = regs->resp0;
        break;

    case SD_CMD_RSPNS_TYPE_136:
        last_r0 = regs->resp0;
        last_r1 = regs->resp1;
        last_r2 = regs->resp2;
        last_r3 = regs->resp3;
        break;
    }

    // If with data, wait for the appropriate interrupt
    if(cmd_reg & SD_CMD_ISDATA)
    {
        uint32_t wr_irpt;
        int is_write = 0;
        if(cmd_reg & SD_CMD_DAT_DIR_CH)
            wr_irpt = (1 << 5);     // read
        else
        {
            is_write = 1;
            wr_irpt = (1 << 4);     // write
        }

        int cur_block = 0;
        uint32_t *cur_buf_addr = (uint32_t *)buf;
        while(cur_block < blocks_to_transfer)
        {
            if(blocks_to_transfer > 1)
                //printf("[EMMC] multi block transfer, awaiting block %d ready\n",
//                       cur_block);
            TIMEOUT_WAIT(regs->interrupt & (wr_irpt | 0x8000), timeout);
            irpts = regs->interrupt;
            regs->interrupt = 0xffff0000 | wr_irpt;

            if((irpts & (0xffff0000 | wr_irpt)) != wr_irpt)
            {
                //printf("[EMMC] error occured whilst waiting for data ready interrupt\n");
                last_error = irpts & 0xffff0000;
                last_interrupt = irpts;
                return;
            }

            // Transfer the block
            size_t cur_byte_no = 0;
            while(cur_byte_no < block_size)
            {
                if(is_write)
                {
                    regs->data = *cur_buf_addr;
                }
                else
                {
                    *cur_buf_addr = regs->data;
                }
                cur_byte_no += 4;
                cur_buf_addr++;
            }

            //printf("[EMMC] block %d transfer complete\n", cur_block);

            cur_block++;
        }
    }

    // Wait for transfer complete (set if read/write transfer or with busy)
    if((((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) ||
            (cmd_reg & SD_CMD_ISDATA)))
    {
        // First check command inhibit (DAT) is not already 0
        if((regs->status & 0x2) == 0)
            regs->interrupt = 0xffff0002;
        else
        {
            TIMEOUT_WAIT(regs->interrupt & 0x8002, timeout);
            irpts = regs->interrupt;
            regs->interrupt = 0xffff0002;

            // Handle the case where both data timeout and transfer complete
            //  are set - transfer complete overrides data timeout: HCSS 2.2.17
            if(((irpts & 0xffff0002) != 0x2) && ((irpts & 0xffff0002) != 0x100002))
            {
                //printf("[EMMC] error occured whilst waiting for transfer complete interrupt\n");
                last_error = irpts & 0xffff0000;
                last_interrupt = irpts;
                return;
            }
            regs->interrupt = 0xffff0002;
        }
    }

    // Return success
    last_cmd_success = 1;
}

static void sd_handle_card_interrupt()
{
    // Handle a card interrupt

//    uint32_t status = regs->status;

    //printf("[EMMC] card interrupt\n");
    //printf("[EMMC] controller status: %08X\n", status);

    // Get the card status
    if(card_rca)
    {
        sd_issue_command_int(sd_commands[SEND_STATUS], card_rca << 16, 500000);
        if(FAIL)
        {
            //printf("[EMMC] unable to get card status\n");
        }
        else
        {
            //printf("[EMMC] card status: %08X\n", last_r0);
        }
    }
    else
    {
        //printf("[EMMC] no card currently selected\n");
    }
}

static void sd_handle_interrupts()
{
    uint32_t irpts = regs->interrupt;
    uint32_t reset_mask = 0;

    if(irpts & SD_COMMAND_COMPLETE)
    {
        //printf("[EMMC] spurious command complete interrupt\n");
        reset_mask |= SD_COMMAND_COMPLETE;
    }

    if(irpts & SD_TRANSFER_COMPLETE)
    {
        //printf("[EMMC] spurious transfer complete interrupt\n");
        reset_mask |= SD_TRANSFER_COMPLETE;
    }

    if(irpts & SD_BLOCK_GAP_EVENT)
    {
        //printf("[EMMC] spurious block gap event interrupt\n");
        reset_mask |= SD_BLOCK_GAP_EVENT;
    }

    if(irpts & SD_DMA_INTERRUPT)
    {
        //printf("[EMMC] spurious DMA interrupt\n");
        reset_mask |= SD_DMA_INTERRUPT;
    }

    if(irpts & SD_BUFFER_WRITE_READY)
    {
        //printf("[EMMC] spurious buffer write ready interrupt\n");
        reset_mask |= SD_BUFFER_WRITE_READY;
        sd_reset_dat();
    }

    if(irpts & SD_BUFFER_READ_READY)
    {
        //printf("[EMMC] spurious buffer read ready interrupt\n");
        reset_mask |= SD_BUFFER_READ_READY;
        sd_reset_dat();
    }

    if(irpts & SD_CARD_INSERTION)
    {
        //printf("[EMMC] card insertion detected\n");
        reset_mask |= SD_CARD_INSERTION;
    }

    if(irpts & SD_CARD_REMOVAL)
    {
        //printf("[EMMC] card removal detected\n");
        reset_mask |= SD_CARD_REMOVAL;
        card_removal = 1;
    }

    if(irpts & SD_CARD_INTERRUPT)
    {
        //printf("[EMMC] card interrupt detected\n");
        sd_handle_card_interrupt();
        reset_mask |= SD_CARD_INTERRUPT;
    }

    if(irpts & 0x8000)
    {
        //printf("[EMMC] spurious error interrupt: %08X\n", irpts);
        reset_mask |= 0xffff0000;
    }

    regs->interrupt = reset_mask;
}

static void sd_issue_command(uint32_t command, uint32_t argument, size_t timeout)
{
    // First, handle any pending interrupts
    sd_handle_interrupts();

    // Stop the command issue if it was the card remove interrupt that was
    //  handled
    if(card_removal)
    {
        last_cmd_success = 0;
        return;
    }

    // Now run the appropriate commands by calling sd_issue_command_int()
    if(command & IS_APP_CMD)
    {
        command &= 0xff;
        //printf("[EMMC] issuing command ACMD%d\n", command);

        if(sd_acommands[command] == SD_CMD_RESERVED(0))
        {
            //printf("[EMMC] invalid command ACMD%d\n", command);
            last_cmd_success = 0;
            return;
        }
        last_cmd = APP_CMD;

        uint32_t rca = 0;
        if(card_rca)
            rca = card_rca << 16;
        sd_issue_command_int(sd_commands[APP_CMD], rca, timeout);
        if(last_cmd_success)
        {
            last_cmd = command | IS_APP_CMD;
            sd_issue_command_int(sd_acommands[command], argument, timeout);
        }
    }
    else
    {
        //printf("[EMMC] issuing command CMD%d\n", command);

        if(sd_commands[command] == SD_CMD_RESERVED(0))
        {
            //printf("[EMMC] invalid command CMD%d\n", command);
            last_cmd_success = 0;
            return;
        }

        last_cmd = command;
        sd_issue_command_int(sd_commands[command], argument, timeout);
    }

    if(FAIL)
    {
        //printf("[EMMC] error issuing command: interrupts %08X: ", last_interrupt);
        if(last_error == 0) 
        {
            //printf("TIMEOUT");
        }
        else
        {
            for(int i = 0; i < SD_ERR_RSVD; i++)
            {
                if(last_error & (1 << (i + 16)))
                {
                    //printf(err_irpts[i]);
                    //printf(" ");
                }
            }
        }
        //printf("\n");
    }
    else
    {
        //printf("[EMMC] command completed successfully\n");
    }
}

int sd_card_init()
{
    //printf("[EMMC] ONE TIME INIT ONLY!\n");
    // Power cycle the card to ensure its in its startup state
    bcm_2708_power_cycle();

    // Read the controller version
    uint32_t ver = regs->slotisr_ver;
    uint32_t sdversion = (ver >> 16) & 0xff;
    //printf("[EMMC] vendor %X, sdversion %X, slot_status %X\n", vendor, sdversion, slot_status);

    if(sdversion < 2)
    {
        //printf("[EMMC] only SDHCI versions >= 3.0 are supported\n");
        return -1;
    }

    // Reset the controller
    //printf("[EMMC] resetting controller\n");
    uint32_t control1 = regs->control1;
    control1 |= (1 << 24);
    // Disable clock
    control1 &= ~(1 << 2);
    control1 &= ~(1 << 0);
    regs->control1 = control1;
    TIMEOUT_WAIT((regs->control1 & (0x7 << 24)) == 0, 1000000);
    if((regs->control1 & (0x7 << 24)) != 0)
    {
        //printf("[EMMC] controller did not reset properly\n");
        return -1;
    }
    //printf("[EMMC] control0: %08X, control1: %08X, control2: %08X\n",
      //     regs->control0,
       //    regs->control1,
        //   regs->control2);

    // Read the capabilities registers
    //printf("[EMMC] capabilities: %08X%08X\n", regs->capabilities1, regs->capabilities0);

    // Check for a valid card
    //printf("[EMMC] checking for an inserted card\n");
    TIMEOUT_WAIT(regs->status & (1 << 16), 500000);
    uint32_t status_reg = regs->status;
    if((status_reg & (1 << 16)) == 0)
    {
        //printf("[EMMC] no card inserted\n");
        return -1;
    }
    //printf("[EMMC] status: %08X\n", status_reg);

    // Clear control2
    regs->control2 = 0;

    // Get the base clock rate
    uint32_t base_clock = sd_get_base_clock_hz();
    if(base_clock == 0)
    {
        //printf("[EMMC] assuming clock rate to be 100MHz\n");
        base_clock = 100000000;
    }

    // Set clock rate to something slow
    //printf("[EMMC] setting clock rate\n");
    regs->control1 = regs->control1 | 1;      // enable clock

    // Set to identification frequency (400 kHz)
    uint32_t f_id = sd_get_clock_divider(base_clock, SD_CLOCK_ID);
    if(f_id == SD_GET_CLOCK_DIVIDER_FAIL)
    {
        //printf("[EMMC] unable to get a valid clock divider for ID frequency\n");
        return -1;
    }
    control1 |= f_id;

    control1 |= (7 << 16);    // data timeout = TMCLK * 2^10
    regs->control1 = control1;
    TIMEOUT_WAIT(regs->control1 & 0x2, 0x1000000);
    if((regs->control1 & 0x2) == 0)
    {
        //printf("[EMMC] controller's clock did not stabilise within 1 second\n");
        return -1;
    }
    //printf("[EMMC] control0: %08X, control1: %08X\n",
      //     regs->control0,
       //    regs->control1);

    // Enable the SD clock
    //printf("[EMMC] enabling SD clock\n");
    wait_us(2000);
    regs->control1 = regs->control1 | 4;
    wait_us(2000);
    //printf("[EMMC] SD clock enabled\n");

    // Mask off sending interrupts to the ARM
    regs->interrupt_enable = 0;
    // Reset interrupts
    regs->interrupt = 0xFFFFFFFF;
    // Have all interrupts sent to the INTERRUPT register
    uint32_t irpt_mask = 0xffffffff & (~SD_CARD_INTERRUPT);
    regs->interrupt_mask = irpt_mask;

    //printf("[EMMC] interrupts disabled\n");
    wait_us(2000);

    block_size = 512;

    //printf("[EMMC] device structure created\n");

    // Send CMD0 to the card (reset to idle state)
    sd_issue_command(GO_IDLE_STATE, 0, 500000);
    if(FAIL)
    {
        //printf("[EMMC] no CMD0 response\n");
        return -1;
    }

    // Send CMD8 to the card
    // Voltage supplied = 0x1 = 2.7-3.6V (standard)
    // Check pattern = 10101010b (as per PLSS 4.3.13) = 0xAA
    //printf("[EMMC] note a timeout error on the following command (CMD8) is normal "
      //     "and expected if the SD card version is less than 2.0\n");
    sd_issue_command(SEND_IF_COND, 0x1aa, 500000);
    int v2_later = 0;
    if(TIMEOUT)
        v2_later = 0;
    else if(CMD_TIMEOUT)
    {
        if(sd_reset_cmd() == -1)
            return -1;
        regs->interrupt = SD_ERR_MASK_CMD_TIMEOUT;
        v2_later = 0;
    }
    else if(FAIL)
    {
        //printf("[EMMC] failure sending CMD8 (%08X)\n", last_interrupt);
        return -1;
    }
    else
    {
        if((last_r0 & 0xfff) != 0x1aa)
        {
            //printf("[EMMC] unusable card\n");
            //printf("[EMMC] CMD8 response %08X\n", last_r0);
            return -1;
        }
        else
            v2_later = 1;
    }

    // Here we are supposed to check the response to CMD5 (HCSS 3.6)
    // It only returns if the card is a SDIO card
    //printf("[EMMC] note that a timeout error on the following command (CMD5) is "
      //     "normal and expected if the card is not a SDIO card.\n");
    sd_issue_command(IO_SET_OP_COND, 0, 10000);
    if(!TIMEOUT)
    {
        if(CMD_TIMEOUT)
        {
            if(sd_reset_cmd() == -1)
                return -1;
            regs->interrupt = SD_ERR_MASK_CMD_TIMEOUT;
        }
        else
        {
            //printf("[EMMC] SDIO card detected - not currently supported\n");
            //printf("[EMMC] CMD5 returned %08X\n", last_r0);
            return -1;
        }
    }

    // Call an inquiry ACMD41 (voltage window = 0) to get the OCR
    //printf("[EMMC] sending inquiry ACMD41\n");
    sd_issue_command(ACMD(41), 0, 500000);
    if(FAIL)
    {
        //printf("[EMMC] inquiry ACMD41 failed\n");
        return -1;
    }
    //printf("[EMMC] inquiry ACMD41 returned %08X\n", last_r0);

    // Call initialization ACMD41
    int card_is_busy = 1;
    while(card_is_busy)
    {
        uint32_t v2_flags = 0;
        if(v2_later)
        {
            // Set SDHC support
            v2_flags |= (1 << 30);

            // Set 1.8v support
            if(!failed_voltage_switch)
                v2_flags |= (1 << 24);

            v2_flags |= (1 << 28);
        }

        sd_issue_command(ACMD(41), 0x00ff8000 | v2_flags, 500000);
        if(FAIL)
        {
            //printf("[EMMC] error issuing ACMD41\n");
            return -1;
        }

        if((last_r0 >> 31) & 0x1)
        {
            // Initialization is complete
            card_ocr = (last_r0 >> 8) & 0xffff;
            card_supports_sdhc = (last_r0 >> 30) & 0x1;

            if(!failed_voltage_switch)
                card_supports_18v = (last_r0 >> 24) & 0x1;

            card_is_busy = 0;
        }
        else
        {
            // Card is still busy
            //printf("[EMMC] card is busy, retrying\n");
            wait_us(500000);
        }
    }

    //printf("[EMMC] card identified: OCR: %04x, 1.8v support: %d, SDHC support: %d\n",
      //     card_ocr, card_supports_18v, card_supports_sdhc);

    // At this point, we know the card is definitely an SD card, so will definitely
    //  support SDR12 mode which runs at 25 MHz
    sd_switch_clock_rate(base_clock, SD_CLOCK_NORMAL);

    // A small wait before the voltage switch
    wait_us(5000);

    // Switch to 1.8V mode if possible
    if(card_supports_18v)
    {
        //printf("[EMMC] switching to 1.8V mode\n");
        // As per HCSS 3.6.1

        // Send VOLTAGE_SWITCH
        sd_issue_command(VOLTAGE_SWITCH, 0, 500000);
        if(FAIL)
        {
            //printf("[EMMC] error issuing VOLTAGE_SWITCH\n");
            failed_voltage_switch = true;
            sd_power_off();
            return sd_card_init();
        }

        // Disable SD clock
        regs->control1 = regs->control1 & ~(1 << 2);

        // Check DAT[3:0]
        status_reg = regs->status;
        uint32_t dat30 = (status_reg >> 20) & 0xf;
        if(dat30 != 0)
        {
            //printf("[EMMC] DAT[3:0] did not settle to 0\n");
            failed_voltage_switch = true;
            sd_power_off();
            return sd_card_init();
        }

        // Set 1.8V signal enable to 1
        regs->control0 = regs->control0 | (1 << 8);

        // Wait 5 ms
        wait_us(5000);

        // Check the 1.8V signal enable is set
        if((regs->control0 & 0x100) == 0)
        {
            //printf("[EMMC] controller did not keep 1.8V signal enable high\n");
            failed_voltage_switch = true;
            sd_power_off();
            return sd_card_init();
        }

        // Re-enable the SD clock
        regs->control1 = regs->control1 | (1 << 2);

        // Wait 1 ms
        wait_us(10000);

        // Check DAT[3:0]
        status_reg = regs->status;
        dat30 = (status_reg >> 20) & 0xf;
        if(dat30 != 0xf)
        {
            //printf("[EMMC] DAT[3:0] did not settle to 1111b (%01x)\n", dat30);
            failed_voltage_switch = true;
            sd_power_off();
            return sd_card_init();
        }

        //printf("[EMMC] voltage switch complete\n");
    }

    // Send CMD2 to get the cards CID
    sd_issue_command(ALL_SEND_CID, 0, 500000);
    if(FAIL)
    {
        //printf("[EMMC] error sending ALL_SEND_CID\n");
        return -1;
    }
/*
    uint32_t card_cid_0 = last_r0;
    uint32_t card_cid_1 = last_r1;
    uint32_t card_cid_2 = last_r2;
    uint32_t card_cid_3 = last_r3;
*/
    //printf("[EMMC] card CID: %08X%08X%08X%08X\n", card_cid_3, card_cid_2, card_cid_1, card_cid_0);
    // Send CMD3 to enter the data state
    sd_issue_command(SEND_RELATIVE_ADDR, 0, 500000);
    if(FAIL)
    {
        //printf("[EMMC] error sending SEND_RELATIVE_ADDR\n");
        return -1;
    }

    uint32_t cmd3_resp = last_r0;
    //printf("[EMMC] CMD3 response: %08X\n", cmd3_resp);

    card_rca = (cmd3_resp >> 16) & 0xffff;
    uint32_t crc_error = (cmd3_resp >> 15) & 0x1;
    uint32_t illegal_cmd = (cmd3_resp >> 14) & 0x1;
    uint32_t error = (cmd3_resp >> 13) & 0x1;
    uint32_t status = (cmd3_resp >> 9) & 0xf;
    uint32_t ready = (cmd3_resp >> 8) & 0x1;

    if(crc_error)
    {
        //printf("[EMMC] CRC error\n");
        return -1;
    }

    if(illegal_cmd)
    {
        //printf("[EMMC] illegal command\n");
        return -1;
    }

    if(error)
    {
        //printf("[EMMC] generic error\n");
        return -1;
    }

    if(!ready)
    {
        //printf("[EMMC] not ready for data\n");
        return -1;
    }

    //printf("[EMMC] RCA: %04x\n", card_rca);

    // Now select the card (toggles it to transfer state)
    sd_issue_command(SELECT_CARD, card_rca << 16, 500000);
    if(FAIL)
    {
        //printf("[EMMC] error sending CMD7\n");
        return -1;
    }

    uint32_t cmd7_resp = last_r0;
    status = (cmd7_resp >> 9) & 0xf;

    if((status != 3) && (status != 4))
    {
        //printf("[EMMC] invalid status (%d)\n", status);
        return -1;
    }

    // If not an SDHC card, ensure BLOCKLEN is 512 bytes
    if(!card_supports_sdhc)
    {
        sd_issue_command(SET_BLOCKLEN, 512, 500000);
        if(FAIL)
        {
            //printf("[EMMC] error sending SET_BLOCKLEN\n");
            return -1;
        }
    }
    block_size = 512;
    uint32_t controller_block_size = regs->blksizecnt;
    controller_block_size &= (~0xfff);
    controller_block_size |= 0x200;
    regs->blksizecnt = controller_block_size;

    // Get the cards SCR register
    buf = &scr.scr[0];
    block_size = 8;
    blocks_to_transfer = 1;
    sd_issue_command(SEND_SCR, 0, 500000);
    block_size = 512;
    if(FAIL)
    {
        //printf("[EMMC] error sending SEND_SCR\n");
        return -1;
    }

    // Determine card version
    // Note that the SCR is big-endian
    uint32_t scr0 = scr.scr[0];
    scr.sd_version = SD_VER_UNKNOWN;
    uint32_t sd_spec = (scr0 >> 0) & 0xf;
    uint32_t sd_spec3 = (scr0 >> 23) & 0x1;
    uint32_t sd_spec4 = (scr0 >> 18) & 0x1;
    scr.sd_bus_widths = (scr0 >> 8) & 0xf;
    if(sd_spec == 0)
        scr.sd_version = SD_VER_1;
    else if(sd_spec == 1)
        scr.sd_version = SD_VER_1_1;
    else if(sd_spec == 2)
    {
        if(sd_spec3 == 0)
            scr.sd_version = SD_VER_2;
        else if(sd_spec3 == 1)
        {
            if(sd_spec4 == 0)
                scr.sd_version = SD_VER_3;
            else if(sd_spec4 == 1)
                scr.sd_version = SD_VER_4;
        }
    }

    //printf("[EMMC] &scr: %08X\n", &scr.scr[0]);
    //printf("[EMMC] SCR[0]: %08X, SCR[1]: %08X\n", scr.scr[0], scr.scr[1]);;
    //printf("[EMMC] SCR (reversed): %08X%08X\n", scr.scr[1], scr.scr[0]);
    //printf("[EMMC] SCR: version %s, bus_widths %01x\n", sd_versions[scr.sd_version],
      //     scr.sd_bus_widths);

    if(scr.sd_bus_widths & 0x4)
    {
        // Set 4-bit transfer mode (ACMD6)
        // See HCSS 3.4 for the algorithm
        //printf("[EMMC] switching to 4-bit data mode\n");

        // Disable card interrupt in host
        uint32_t old_irpt_mask = regs->interrupt_mask;
        uint32_t new_iprt_mask = old_irpt_mask & ~(1 << 8);
        regs->interrupt_mask = new_iprt_mask;

        // Send ACMD6 to change the card's bit mode
        sd_issue_command(SET_BUS_WIDTH, 0x2, 500000);
        if(FAIL)
        {
            //printf("[EMMC] switch to 4-bit data mode failed\n");
        }
        else
        {
            // Change bit mode for Host
            regs->control0 = regs->control0 | 0x2;

            // Re-enable card interrupt in host
            regs->interrupt_mask = old_irpt_mask;

            //printf("[EMMC] switch to 4-bit complete\n");
        }
    }

    //printf("[EMMC] found a valid version %s SD card\n", sd_versions[scr.sd_version]);
    //printf("[EMMC] setup successful (status %d)\n", status);

    // Reset interrupt register
    regs->interrupt = 0xffffffff;

    return 0;
}

static int sd_ensure_data_mode()
{
    if(card_rca == 0)
    {
        // Try again to initialise the card
        int ret = sd_card_init();
        if(ret != 0)
            return ret;
    }

    //printf("[EMMC] ensure_data_mode() obtaining status register for card_rca %08X: ",
      //     card_rca);

    sd_issue_command(SEND_STATUS, card_rca << 16, 500000);
    if(FAIL)
    {
        //printf("[EMMC] ensure_data_mode() error sending CMD13\n");
        card_rca = 0;
        return -1;
    }

    uint32_t status = last_r0;
    uint32_t cur_state = (status >> 9) & 0xf;
    //printf("status %d\n", cur_state);
    if(cur_state == 3)
    {
        // Currently in the stand-by state - select it
        sd_issue_command(SELECT_CARD, card_rca << 16, 500000);
        if(FAIL)
        {
            //printf("[EMMC] ensure_data_mode() no response from CMD17\n");
            card_rca = 0;
            return -1;
        }
    }
    else if(cur_state == 5)
    {
        // In the data transfer state - cancel the transmission
        sd_issue_command(STOP_TRANSMISSION, 0, 500000);
        if(FAIL)
        {
            //printf("[EMMC] ensure_data_mode() no response from CMD12\n");
            card_rca = 0;
            return -1;
        }

        // Reset the data circuit
        sd_reset_dat();
    }
    else if(cur_state != 4)
    {
        // Not in the transfer state - re-initialise
        int ret = sd_card_init();
        if(ret != 0)
            return ret;
    }

    // Check again that we're now in the correct mode
    if(cur_state != 4)
    {
        //printf("[EMMC] ensure_data_mode() rechecking status: ");
        sd_issue_command(SEND_STATUS, card_rca << 16, 500000);
        if(FAIL)
        {
            //printf("[EMMC] ensure_data_mode() no response from CMD13\n");
            card_rca = 0;
            return -1;
        }
        status = last_r0;
        cur_state = (status >> 9) & 0xf;

        //printf("%d\n", cur_state);

        if(cur_state != 4)
        {
            //printf("[EMMC] unable to initialise SD card to "
              //     "data mode (state %d)\n", cur_state);
            card_rca = 0;
            return -1;
        }
    }

    return 0;
}

static int sd_do_data_command(int is_write, uint8_t *buf, size_t buf_size, uint32_t block_no)
{
    if(!card_supports_sdhc)
        block_no *= 512;     // SD uses byte addresses


    blocks_to_transfer = buf_size / block_size;
    if(buf_size % block_size)
    {
        //printf("[EMMC] do_data_command() called with buffer size (%d) not an "
          //     "exact multiple of block size (%d)\n", buf_size, block_size);
        return -1;
    }
    if (blocks_to_transfer == 0)
    {
        //printf("[EMMC] do_data_command() called to do nothing!\n");
        return -1;
    }
    ::buf = buf;

    // Decide on the command to use
    int command;
    if(is_write)
    {
        if(blocks_to_transfer > 1)
            command = WRITE_MULTIPLE_BLOCK;
        else
            command = WRITE_BLOCK;
    }
    else
    {
        if(blocks_to_transfer > 1)
            command = READ_MULTIPLE_BLOCK;
        else
            command = READ_SINGLE_BLOCK;
    }

    int retry_count = 0;
    int max_retries = 3;
    while(retry_count < max_retries)
    {
        sd_issue_command(command, block_no, 5000000);

        if(SUCCESS)
            break;
        else
        {
            //printf("[EMMC] error sending CMD%d, ", command);
            //printf("error = %08X.  ", last_error);
            retry_count++;
            if(retry_count < max_retries) {
                //printf("Retrying...\n");
            } else {
                //printf("Giving up.\n");
            }
        }
    }
    if(retry_count == max_retries)
    {
        card_rca = 0;
        return -1;
    }

    return 0;
}

int sd_read(uint8_t *buf, size_t buf_size, uint32_t block_no)
{
    // Check the status of the card
    if(sd_ensure_data_mode() != 0)
        return -1;

    //printf("[EMMC] read() card ready, reading from block %u\n", block_no);

    if(sd_do_data_command(0, buf, buf_size, block_no) < 0)
        return -1;

    //printf("[EMMC] data read successful\n");

    return buf_size;
}

int sd_write(uint8_t *buf, size_t buf_size, uint32_t block_no)
{
    // Check the status of the card
    if(sd_ensure_data_mode() != 0)
        return -1;

    //printf("[EMMC] write() card ready, reading from block %u\n", block_no);

    if(sd_do_data_command(1, buf, buf_size, block_no) < 0)
        return -1;

    //printf("[EMMC] write read successful\n");

    return buf_size;
}

RaspberryPi::SdCard::SdCard() {
  sd_card_init();
  register_storage(this);
}

DiskBlock* RaspberryPi::SdCard::readBlock(size_t sector, size_t count) {
  DiskBlock *db = new DiskBlock(count*512);
  sd_read(db->data, db->size, sector);
  return db;
}


