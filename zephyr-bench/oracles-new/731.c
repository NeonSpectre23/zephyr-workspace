static int setup(int *sock, struct sockaddr *bind_addr,
		 socklen_t bind_addrlen)
{
	int ret;

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
	*sock = socket(bind_addr->sa_family, SOCK_STREAM, IPPROTO_TLS_1_2);
#else
	*sock = socket(bind_addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
#endif
	if (*sock < 0) {
		LOG_ERR("Failed to create TCP socket: %d", errno);
		return -errno;
	}

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
	sec_tag_t sec_tag_list[] = {
		SERVER_CERTIFICATE_TAG,
	};

	ret = setsockopt(*sock, SOL_TLS, TLS_SEC_TAG_LIST,
			 sec_tag_list, sizeof(sec_tag_list));
	if (ret < 0) {
		LOG_ERR("Failed to set TCP secure option %d", errno);
		ret = -errno;
	}
#endif

	ret = bind(*sock, bind_addr, bind_addrlen);
	if (ret < 0) {
		LOG_ERR("Failed to bind TCP socket %d", errno);
		return -errno;
	}

	ret = listen(*sock, MAX_CLIENT_QUEUE);
	if (ret < 0) {
		LOG_ERR("Failed to listen on TCP socket %d", errno);
		ret = -errno;
	}

	return ret;
}