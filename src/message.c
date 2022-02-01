extern struct event_loop g_event_loop;

static struct {
    int sockfd;
    bool is_running;
    pthread_t thread;
} g_message_loop;

struct token
{
    const char *text;
    unsigned int length;
};

static struct token get_token(char **message)
{
    struct token token;

    token.text = *message;
    while (**message && !isspace(**message)) {
        ++(*message);
    }
    token.length = *message - token.text;

    if (isspace(**message)) {
        ++(*message);
    } else {
        // NOTE(koekeishiya): don't go past the null-terminator
    }

    return token;
}

static bool token_equals(struct token token, const char *match)
{
    const char *at = match;
    for (int i = 0; i < token.length; ++i, ++at) {
        if ((*at == 0) || (token.text[i] != *at)) {
            return false;
        }
    }
    return *at == 0;
}

static uint32_t token_to_uint32t(struct token token)
{
    uint32_t result = 0;
    char buffer[token.length + 1];
    memcpy(buffer, token.text, token.length);
    buffer[token.length] = '\0';
    sscanf(buffer, "%d", &result);
    return result;
}

static void handle_message(char *message)
{
    struct token token = get_token(&message);
    if (token_equals(token, "create")) {
        struct token wid_token = get_token(&message);
        uint32_t wid = token_to_uint32t(wid_token);
        event_loop_post(&g_event_loop, WINDOW_CREATED, (void *) (intptr_t) wid, 0, NULL);
    } else if (token_equals(token, "destroy")) {
        struct token wid_token = get_token(&message);
        uint32_t wid = token_to_uint32t(wid_token);
        event_loop_post(&g_event_loop, WINDOW_DESTROYED, (void *) (intptr_t) wid, 0, NULL);
    } else if (token_equals(token, "mission_control_enter")) {
        event_loop_post(&g_event_loop, WINDOW_MISSION_CONTROL_ENTER, NULL, 0, NULL);
    } else if (token_equals(token, "mission_control_exit")) {
        event_loop_post(&g_event_loop, WINDOW_MISSION_CONTROL_EXIT, NULL, 0, NULL);
    } else if (token_equals(token, "query")) {
        event_loop_post(&g_event_loop, WINDOW_QUERY_YABAI, NULL, 0, NULL);
    }
}

static bool read_message(int sockfd, char *message, size_t length)
{
    int len = recv(sockfd, message, length, 0);
    if (len <= 0) return false;

    message[len] = '\0';
    return true;
}


static void *message_loop_run(void *context)
{
    while (g_message_loop.is_running) {
        int sockfd = accept(g_message_loop.sockfd, NULL, 0);
        if (sockfd == -1) continue;

        char message[255];
        if (read_message(sockfd, message, 254)) {
            handle_message(message);
        }

        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
    }

    return NULL;
}

bool message_loop_begin(char *socket_path)
{
    struct sockaddr_un socket_address;
    socket_address.sun_family = AF_UNIX;
    snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
    unlink(socket_path);

    if ((g_message_loop.sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    if (bind(g_message_loop.sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) == -1) {
        return false;
    }

    if (chmod(socket_path, 0600) != 0) {
        return false;
    }

    if (listen(g_message_loop.sockfd, SOMAXCONN) == -1) {
        return false;
    }

    fcntl(g_message_loop.sockfd, F_SETFD, FD_CLOEXEC | fcntl(g_message_loop.sockfd, F_GETFD));

    g_message_loop.is_running = true;
    pthread_create(&g_message_loop.thread, NULL, &message_loop_run, NULL);

    return true;
}
