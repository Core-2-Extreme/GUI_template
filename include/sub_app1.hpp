#if !defined(SAPP1_HPP)
#define SAPP1_HPP

#include "system/types.hpp"
#include "system/util/hid.hpp"

bool Sapp1_query_init_flag(void);

bool Sapp1_query_running_flag(void);

void Sapp1_hid(Hid_info key);

void Sapp1_resume(void);

void Sapp1_suspend(void);

Result_with_string Sapp1_load_msg(std::string lang);

void Sapp1_init(bool draw);

void Sapp1_exit(bool draw);

void Sapp1_main(void);

#endif //!defined(SAPP1_HPP)
