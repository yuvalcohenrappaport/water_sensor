#ifndef EMAIL_STUBS_H
#define EMAIL_STUBS_H

// This header provides stub implementations for ESP_Mail_Client
// when the library is not available (e.g., on macOS ARM64)

#ifndef ESP_MAIL_CLIENT_H
#define ESP_MAIL_CLIENT_H

// Enum stub
enum Content_Transfer_Encoding {
  enc_7bit = 0
};

// Session stub
struct ESP_Mail_Session {
  struct {
    const char* host_name;
    int port;
  } server;
  struct {
    const char* email;
    const char* password;
    const char* user_domain;
  } login;
};

// Message stub
struct SMTP_Message {
  struct {
    const char* name;
    const char* email;
  } sender;
  const char* subject;
  struct {
    const char* content;
    const char* charSet;
    int transfer_encoding;
  } html;
  struct {
    const char* charSet;
  } text;
  
  void addRecipient(const char* email) {}
};

// Session stub
struct SMTPSession {
  void debug(int level) {}
  void setCallback(void (*callback)()) {}
  const char* statusMessage() { return "Email stub (testing mode)"; }
};

// MailClient stub
struct MailClientStub {
  bool sendEmail(SMTPSession& session, SMTP_Message& message) {
    // On macOS/testing: simulate successful send
    return true;  // Return true to indicate "successful" send
  }
};

static MailClientStub MailClient;

#endif // ESP_MAIL_CLIENT_H
#endif // EMAIL_STUBS_H
