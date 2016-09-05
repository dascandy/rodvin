#include "xhci.h"
#include <stdio.h>
#include "pci.h"
#include "platform.h"
#include "future.h"

#define VIRTUAL_ADDRESS 0xFFFFFF00FFFFF000

struct capregs {
  uint16_t caplength;
  uint16_t hciversion;
  uint32_t hcsparams1;
  uint32_t hcsparams2;
  uint32_t hcsparams3;
  uint32_t hccparams;
  uint32_t dboff;
  uint32_t rtsoff;
};

struct opregs {
  uint32_t usbcmd;
  uint32_t usbsts;
  uint32_t pagesize;
  uint32_t pad0[2];
  uint32_t dnctrl;
  uint64_t crcr;
  uint32_t pad1[4];
  uint64_t dcbaap;
  uint32_t config;
};

struct portregs {
  uint32_t portsc;
  uint32_t portpmsc;
  uint32_t portli;
  uint32_t reserved;
};

struct rtregs {

};

Xhci::Xhci(pcidevice dev)
: dev(dev)
{
  uint64_t ptr = ((uint64_t)pciread32(dev, 0x14) << 32) | pciread32(dev, 0x10);
  printf("Found xhci device! %p\n", ptr);
  platform_map((void*)VIRTUAL_ADDRESS, ptr, DeviceRegisters);
  volatile capregs* cr = (capregs*)(VIRTUAL_ADDRESS);
  volatile opregs* opr = (opregs*)(VIRTUAL_ADDRESS + cr->caplength);
  volatile portregs* pr = (portregs*)(VIRTUAL_ADDRESS + 0x400);
/*

  volatile rtregs* rr = (rtregs*)(VIRTUAL_ADDRESS + cr->rtsoff);
  volatile uint32_t* doorbell = (uint32_t*)(VIRTUAL_ADDRESS + cr->dboff);

  uint32_t maxports = (cr->hcsparams1 >> 24) & 0xFF;
  uint32_t maxinterrupters = (cr->hcsparams1 >> 24) & 0x7FF;
  uint32_t maxslots = (cr->hcsparams1 >> 0) & 0xFF;
  printf("%d %d %d\n", maxports, maxinterrupters, maxslots);

  hc_constructRootPorts(&x->hc, BYTE4(x->CapRegs->hcsparams1), &USB_XHCI);
  printf("\nx->hc.rootPortCount: %u", x->hc.rootPortCount);

  uint16_t pciCommandRegister = pci_configRead(x->PCIdevice, PCI_COMMAND, 2);
  pci_configWrite_word(x->PCIdevice, PCI_COMMAND, pciCommandRegister | PCI_CMD_MMIO | PCI_CMD_BUSMASTER);

  x->OpRegs->command &= ~CMD_RUN;
  while((x->OpRegs->status & STS_HCH) == 0) {}

  x->OpRegs->command |= CMD_RESET;
  while((x->OpRegs->command & CMD_RESET) != 0) {}

  deactivateLegacySupport(x);

  x->OpRegs->devnotifctrl = 0x2;

  x->OpRegs->config |= MAX_HC_SLOTS;

  // Program the Device Context Base Address Array Pointer (DCBAAP) register (5.4.6) with a 64-bit address pointing to where the Device Context Base Address Array is located.
  x->virt_deviceContextPointerArrayBase = malloc(sizeof(xhci_DeviceContextArray_t), 64 | HEAP_CONTINUOUS, "xhci_DevContextArray"); // Heap is contiguous below 4K. Alignment see Table 54
  x->OpRegs->dcbaap = (uint64_t)paging_getPhysAddr(x->virt_deviceContextPointerArrayBase);

  uint8_t MaxScratchpadBuffers = ((x->CapRegs->hcsparams2 >> 27) & 0x1F) | ((x->CapRegs->hcsparams2 >> 16) & 0xE0);
  if (MaxScratchpadBuffers > 0) // Max Scratchpad Buffers
  {
    printf("\nscratchpad buffer created! Max Scratchpad Buffers = %u", MaxScratchpadBuffers);
    uint64_t* ScratchpadBuffersPtr = malloc(sizeof(uint64_t)*MaxScratchpadBuffers, 64 | HEAP_CONTINUOUS, "xhci_ScratchpadBuffersPtr");
    for (uint8_t i=0; i<MaxScratchpadBuffers; i++)
    {
      ScratchpadBuffersPtr[i] = paging_getPhysAddr(malloc(PAGESIZE, PAGESIZE | HEAP_CONTINUOUS, "xhci_ScratchpadBuffer"));
    }
    x->virt_deviceContextPointerArrayBase->scratchpadBufferArrBase = (uint64_t)paging_getPhysAddr(ScratchpadBuffersPtr); // Ptr to scratchpad buffer array
  }
  else // Max Scratchpad Buffers = 0
  {
    x->virt_deviceContextPointerArrayBase->scratchpadBufferArrBase = 0;
  }

  // Device Contexts
  for (uint16_t i=0; i<MAX_HC_SLOTS; i++)
  {
    x->devContextPtr[i] = malloc(sizeof(xhci_DeviceContext_t), 64 | HEAP_CONTINUOUS, "xhci_DevContext"); // Alignment see Table 54
    memset(x->devContextPtr[i], 0, sizeof(xhci_DeviceContext_t));
    x->virt_deviceContextPointerArrayBase->devContextPtr[i] = (uintptr_t)paging_getPhysAddr(x->devContextPtr[i]);
  }

  // Input Device Contexts
  for (uint16_t i=0; i<MAX_HC_SLOTS; i++)
  {
    x->devInputContextPtr[i] = malloc(sizeof(xhci_InputContext_t), 64 | HEAP_CONTINUOUS, "xhci_DevInputContext"); // Alignment see Table 54
    memset(x->devInputContextPtr[i], 0, sizeof(xhci_InputContext_t));
  }

  // Transfer Rings
  for (uint16_t slotNr=1; slotNr<=MAX_HC_SLOTS; slotNr++)
  {
    x->slots[slotNr-1] = malloc(sizeof(xhci_slot_t), 0, "xhci_slots");

    for (uint16_t i=0; i<NUM_ENDPOINTS; i++)
    {
      xhci_xfer_NormalTRB_t* trb = malloc(256*sizeof(xhci_xfer_NormalTRB_t), PAGESIZE | HEAP_CONTINUOUS, "xhci_transferTRB"); // Alignment see Table 54. Use PAGESIZE to ensure that the memory is within one page.
      memset(trb, 0, 256 * sizeof(xhci_xfer_NormalTRB_t));
      x->trb[slotNr-1][i] = trb;

      // Software uses and maintains private copies of the Enqueue and Dequeue Pointers for each Transfer Ring.
      x->slots[slotNr-1]->endpoints[i].virtEnqTransferRingPtr = x->slots[slotNr-1]->endpoints[i].virtDeqTransferRingPtr = trb;
      x->slots[slotNr-1]->endpoints[i].TransferRingProducerCycleState = true; // PCS
      x->slots[slotNr-1]->endpoints[i].TransferCounter = 0; // Reset Transfer Counter
      x->slots[slotNr-1]->endpoints[i].timeEvent = 0;
      x->slots[slotNr-1]->endpoints[i].TransferRingbase = trb;

      // LinkTRB
      xhci_LinkTRB_t* linkTrb = (xhci_LinkTRB_t*)(trb + 255);
      linkTrb->RingSegmentPtrLo = paging_getPhysAddr(trb); // segment pointer
      linkTrb->IntTarget = 0;                // intr target
      linkTrb->TC = 1;                   // Toggle Cycle !! 4.11.5.1 Link TRB
      linkTrb->TRBtype = TRB_TYPE_LINK;          // ID of link TRB, cf. table 131
    }
  }

  // Command Ring
  // Define the Command Ring Dequeue Pointer by programming the Command Ring Control Register (5.4.5) with a 64-bit address pointing to the starting address of the first TRB of the Command Ring.
  x->CmdRingbase = malloc(256*sizeof(xhci_LinkTRB_t), 64 | HEAP_CONTINUOUS, "xhci_cmdTRB"); // Alignment see Table 54
  x->virtEnqCmdRingPtr = x->CmdRingbase;
  memset(x->CmdRingbase, 0, 256*sizeof(xhci_LinkTRB_t));

  // LinkTRB
  x->CmdRingbase[255].RingSegmentPtrLo = paging_getPhysAddr(x->CmdRingbase); // segment pointer
  x->CmdRingbase[255].TC = 1;                        // Toggle Cycle
  x->CmdRingbase[255].TRBtype = TRB_TYPE_LINK;                 // ID of link TRB, cf. table 131

  // Command Ring Control Register (5.4.5), "crcr"
  x->CmdRingProducerCycleState = true; // PCS
  x->OpRegs->crcr = paging_getPhysAddr(x->CmdRingbase) | x->CmdRingProducerCycleState; // command ring control register, Table 32, 5.4.5, CCS is Bit 0. Bit5:4 are RsvdP, but cannot be read.

  xhci_configureInterrupts(x);
  xhci_prepareEventRing(x);

  // Write the USBCMD (5.4.1) to turn the host controller ON via setting the Run/Stop (R/S) bit to ���� This operation allows the xHC to begin accepting doorbell references.
  x->OpRegs->command |= CMD_RUN; // set Run/Stop bit
  sleepMilliSeconds(100); //  IMPORTANT

  xhci_showStatus(x);

  if (!(x->OpRegs->status & STS_HCH)) // HC not Halted
  {
    xhci_prepareSlotsForControlTransfers(x);
  }
  else // HC Halted
  {
    printfe("\nFatal Error: HCHalted set. Ports cannot be enabled.");
    xhci_showStatus(x);
  }

  textColor(LIGHT_MAGENTA);
  printf("\n\n>>> Press key to close this console. <<<");
  getch();
}





static void xhci_configureInterrupts(xhci_t* x)
{
  #ifdef _XHCI_DIAGNOSIS_
    textColor(HEADLINE);
    printf("\nconfigureInterrupts");
    textColor(TEXT);
  #endif

    // MSI (We do not support MSI-X)
    x->msiCapEnabled = pci_trySetMSIVector(x->PCIdevice, APICIRQ);

    if (x->msiCapEnabled)
        x->irq = APICIRQ;
    else
        x->irq = x->PCIdevice->irq;
    irq_installPCIHandler(x->irq, xhci_handler, x->PCIdevice);
*/
}

