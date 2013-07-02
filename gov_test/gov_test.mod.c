#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8e8695ce, "module_layout" },
	{ 0x1cb52733, "cpufreq_register_governor" },
	{ 0xadaa2657, "cpufreq_register_notifier" },
	{ 0x62b72b0d, "mutex_unlock" },
	{ 0x238d6c76, "__cpufreq_driver_target" },
	{ 0x3b3016d3, "cpufreq_unregister_notifier" },
	{ 0x27e1a049, "printk" },
	{ 0x2aad5e53, "cpufreq_unregister_governor" },
	{ 0xb4390f9a, "mcount" },
	{ 0xe16b893b, "mutex_lock" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "5EC1B22AE3C104764FAAAA8");
