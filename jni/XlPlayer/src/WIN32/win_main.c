#include "XL_log.h"
#include "XL_rtsp_session.h"
#include "XL_scheduler.h"

//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

static Scheduler *schd;
static RtspSession *session_1;
static RtspSession *session_2;

#undef main
int main(int argc, char* argv[])
{
	schd = create_scheduler();
	initialize_scheduler(schd, NULL);

	session_1 = create_session();
	initialize_session(session_1, "222.214.218.237", 6601, "1299880", 0);
	set_session_index(session_1, 0);
	add_session(schd, session_1);

	session_2 = create_session();
	initialize_session(session_2, "222.214.218.237", 6601, "1299880", 1);
	set_session_index(session_2, 1);
	add_session(schd, session_2);

	set_surface_mode(schd->surface, mode_2);

	scheduler_start(schd);
	session_start(session_1);
	session_start(session_2);

	scheduler_wait(schd);

	session_stop(session_1);
	session_stop(session_2);
	destroy_session(session_1);
	destroy_session(session_2);

	return 0;
}