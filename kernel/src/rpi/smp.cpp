
/*
+int __cpuinit bcm2709_boot_secondary(unsigned int cpu, struct task_struct *idle)
+{
+ void secondary_startup(void);
+ void *mbox_set = __io_address(ARM_LOCAL_MAILBOX3_SET0 + 0x10 * MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 0));
+ void *mbox_clr = __io_address(ARM_LOCAL_MAILBOX3_CLR0 + 0x10 * MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 0));
+ unsigned secondary_boot = (unsigned)virt_to_phys((void *)secondary_startup);
+ int timeout=20;
+ unsigned t = -1;
+ //printk("[%s] enter cpu:%d (%x->%p) %x\n", __FUNCTION__, cpu, secondary_boot, wake, readl(wake));
+
+ dsb();
+ BUG_ON(readl(mbox_clr) != 0);
+ writel(secondary_boot, mbox_set);
+
+ while (--timeout > 0) {
+ t = readl(mbox_clr);
+ if (t == 0) break;
+ cpu_relax();
+ }
+ if (timeout==0)
+ printk("[%s] cpu:%d failed to start (%x)\n", __FUNCTION__, cpu, t);
+ else
+ printk("[%s] cpu:%d started (%x) %d\n", __FUNCTION__, cpu, t, timeout);
+
+ return 0;
+}
*/

