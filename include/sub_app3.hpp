#if !defined(SAPP3_HPP)
#define SAPP3_HPP

#include "system/types.hpp"
#include "system/util/hid.hpp"

bool Sapp3_query_init_flag(void);

bool Sapp3_query_running_flag(void);

void Sapp3_hid(Hid_info key);

void Sapp3_resume(void);

void Sapp3_suspend(void);

Result_with_string Sapp3_load_msg(std::string lang);

void Sapp3_init(bool draw);

void Sapp3_exit(bool draw);

void Sapp3_main(void);

#endif //!defined(SAPP3_HPP)
