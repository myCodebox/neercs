//
// Neercs
//

#if !defined __TERM_PTY_H__
#define __TERM_PTY_H__

class Pty
{
public:
    Pty(ivec2 size);
    ~Pty();

    void Run(char const *command);
    size_t ReadData(char *data, size_t maxlen);
    void UnreadData(char *data, size_t len);
    void SetWindowSize(ivec2 size);

private:
    int64_t m_fd;
    int64_t m_pid;
    char const *m_argv[2];
    char *m_unread_data;
    size_t m_unread_len;
    ivec2 m_size;
};

#endif // __TERM_PTY_H__
