#ifndef crisp_comms_Error_hh
#define crisp_comms_Error_hh 1

namespace crisp
{
  namespace comms
  {
  typedef uint8_t ErrorCategory;
  typedef uint8_t ErrorCode;

  struct Error {
    Error(ErrorCategory category, ErrorCode code, const char* message, const char* reason_or_problem_argument = NULL);
    ~Error();

    ErrorCategory category;
    ErrorCode code;
    const char* message;
    const char* source;
  };
  }
}

#endif	/* crisp_comms_Error_hh */
