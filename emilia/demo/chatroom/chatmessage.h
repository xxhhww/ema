#ifndef _EMILIA_CHATMESSAGE_H_
#define _EMILIA_CHATMESSAGE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
// s -> c   c -> s message { header, body } // header length

class chatMessage {
public:
  enum { max_body_length = 512 };

  chatMessage() : body_length_(0) {}

  const char *data() const { return data_; }

  char *data() { return data_; }

  std::size_t body_length() const { return body_length_; }

  void body_length(std::size_t new_length) {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

  void clear() { bzero(data_, sizeof(data_)); }

private:
  char data_[max_body_length];
  std::size_t body_length_;
};

#endif