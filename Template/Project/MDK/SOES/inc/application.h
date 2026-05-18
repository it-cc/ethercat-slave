#ifndef __APPLICATION_H
#define __APPLICATION_H

/* 应用层初始化函数 */
void app_init(void);

/* SOES 协议栈回调函数（由协议栈调用） */
void cb_get_inputs(void);
void cb_set_outputs(void);

#endif /* __APPLICATION_H */
