/* This is a sample demonstration application that showcases usage of rpmsg 
This application is meant to run on the remote CPU running baremetal code. 
This application echoes back data that was sent to it by the master core. */

#include <stdio.h>
#include <string.h>
#include "open_amp.h"
#include "rsc_table.h"
#include "baremetal.h"
#include "xpseudo_asm_gcc.h"

#define SHUTDOWN_MSG 0xEF56A55A

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *, unsigned long);
static void init_system();

/* Globals */
char DBG_MSG[256] = {1, 2, 3, 4} ; 
static struct rpmsg_channel *app_rp_chnl;
static struct rpmsg_endpoint *rp_ept;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
extern const struct remote_resource_table resources;

/* Application entry point */
int main() {

	
	/* Switch to System Mode */
	SWITCH_TO_SYS_MODE();

	/* Initialize HW system components */
	init_system();

	Xil_DCacheDisable();

	rsc_info.rsc_tab = (struct resource_table *)&resources;
	rsc_info.size = sizeof(resources);
	sprintf(DBG_MSG, "i am in main function.\n");

	remoteproc_resource_init(&rsc_info, rpmsg_channel_created, rpmsg_channel_deleted, rpmsg_read_cb,
		&proc);
	while(1) {
		 __asm__ ( "\
			wfi\n\t" \
		);
	};
	return 0;
}

static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl) {
	app_rp_chnl = rp_chnl;
	rp_ept = rpmsg_create_ept(rp_chnl, rpmsg_read_cb, RPMSG_NULL,
			RPMSG_ADDR_ANY);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl) {
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
		void * priv, unsigned long src) {
	if ((*(int *) data) == SHUTDOWN_MSG) {
		remoteproc_resource_deinit(proc);
	} else {
		/* Send data back to master */
		rpmsg_send(rp_chnl, data, len);
	}
}

static void init_system() {

	/* To Do -- Fix Me later */
	/* Place the vector table -- Do we need it? */
	/* Initialize stacks -- Do we need it? */
	/* Initilaize GIC */
	zynqMP_r5_gic_initialize();
}
