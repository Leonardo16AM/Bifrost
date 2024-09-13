// LLMClient.h
#ifndef LLMSCLIENT_H
#define LLMSCLIENT_H

#include <string>

class LLMClient {
public:
    LLMClient();
    std::string getResponse(const std::string& prompt);

private:
    std::string execCommand(const std::string& cmd);
};

#endif // LLMSCLIENT_H
