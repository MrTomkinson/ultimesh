#pragma once
#include <Arduino.h>

void cmd_ls(const String& args, Print* out);
void cmd_ls_detailed(const String& args, Print* out);
void cmd_cat(const String& args, Print* out);
void cmd_rm(const String& args, Print* out);
void cmd_edit(const String& args, Print* out);
void cmd_touch(const String& args, Print* out);
void cmd_mv(const String& args, Print* out);
void cmd_cp(const String& args, Print* out);
void cmd_echo(const String& args, Print* out);
void cmd_clear(const String& args, Print* out);
void cmd_help(const String& args, Print* out);
void cmd_config(const String& args, Print* out);
void cmd_reload(const String& args, Print* out);
void cmd_tokens(const String& args, Print* out);
void cmd_top(const String& args, Print* out);
void cmd_exit(const String& args, Print* out);
void cmd_reboot(const String& args, Print* out);
void cmd_ping(const String& args, Print* out);
void cmd_version(const String& args, Print* out);
void cmd_whoami(const String& args, Print* out);
