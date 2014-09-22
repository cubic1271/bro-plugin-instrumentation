module Instrumentation;

export {
	const function_profile_log = "/tmp/bro-profile-function" &redef;
	const function_profile_enable = F &redef;
}

event bro_init() {
	if(function_profile_enable) {
		SetFunctionDataTarget(function_profile_log);
	}
}

event bro_done() {
	if(function_profile_enable) {
		FunctionDataWrite();
		FunctionDataFinalize();
	}
}
