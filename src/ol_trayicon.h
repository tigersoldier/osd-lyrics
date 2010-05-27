#ifndef __OL_TRAYICON_H__
#define __OL_TRAYICON_H__

enum OlPlayerStatus;

/** 
 * @brief give a Entrance to do trayicon job
 * 
 */
void ol_trayicon_inital ();

/** 
 * @brief Notifiy the trayicon that the playing status has changed
 * 
 */
void ol_trayicon_status_changed (enum OlPlayerStatus status);

void ol_trayicon_free ();
#endif
