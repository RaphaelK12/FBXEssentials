#ifndef _LOADSHADERS_H
#define _LOADSHADERS_H

void loadShaders(GLuint&prog_hdlr, const char*vsfile, const char*fsfile);
void setShaders(GLuint &prog_hdlr, GLuint vert_hdlr, GLuint frag_hdlr);

#endif //_LOADSHADERS_H