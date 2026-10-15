#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* All the work is done by the threads (button.c, controller.c, measure.c).
 * They start automatically (K_THREAD_DEFINE), so main only prints a banner. */
int main(void)
{
	printk("Embedded Platforms and Communications for IoT\n");
	printk("        ETSIST - UPM - MUIoT 2026-2027       \n\n");
	return 0;
}
