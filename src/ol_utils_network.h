#ifndef _OL_UTILS_NETWORK_H_
#define _OL_UTILS_NETWORK_H_

enum OlProxyType {
  OL_PROXY_NONE,
  OL_PROXY_ENVAR,
  OL_PROXY_HTTP,
  OL_PROXY_SOCKS4,
  OL_PROXY_SOCKS5,
};

enum OlProxyType ol_get_proxy_type ();
char *ol_get_proxy_host ();
int ol_get_proxy_port ();
char *ol_get_proxy_username ();
char *ol_get_proxy_password ();

#endif /* _OL_UTILS_NETWORK_H_ */
