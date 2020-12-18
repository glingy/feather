#ifndef ERROR_H
#define ERROR_H

#ifdef __cplusplus
extern "C" {
#endif   
  void error(const char * line1, const char * line2);

#ifdef __cplusplus
 }
#endif
void error_multiline(const char ** msg, unsigned char len);

#endif