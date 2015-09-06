#include "XL_rtsp_session.h"
#include "XL_scheduler.h"

//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#undef main
int main(int argc, char* argv[])
{
	RtspSession *session_1;
	RtspSession *session_2;
	Scheduler *schd;
	unsigned char cmd;

	schd = create_scheduler();
	initialize_scheduler(schd);

	session_1 = create_session();
	initialize_session(session_1, "222.214.218.237", 6601, 100, 100, 320, 240);
	add_session(schd, session_1);

	session_2 = create_session();
	initialize_session(session_2, "222.214.218.237", 6601, 420, 100, 320, 240);
	add_session(schd, session_2);

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