// LLMClient.cpp
#include "call_llm.h"
#include <iostream>
#include <stdexcept>
#include <array>
#include <memory>
#include <cstdio>
#include <sstream>

LLMClient::LLMClient() {}

std::string LLMClient::execCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    // Abrir un pipe para el comando
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Error al ejecutar el comando.");
    }
    // Leer la salida del comando
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string LLMClient::getResponse(const std::string& prompt) {
    if (prompt.empty()) {
        throw std::invalid_argument("El prompt no puede estar vacío.");
    }

    // Escapar comillas dobles y otros caracteres especiales en el prompt
    std::ostringstream escaped;
    for(char c : prompt){
        if(c == '\"') escaped << "\\\"";
        else if(c == '\\') escaped << "\\\\";
        else escaped << c;
    }
    std::string escaped_prompt = escaped.str();

    // Construir el comando para ejecutar el script de Python
    // Asegúrate de que el script 'call_llm.py' esté en el mismo directorio o proporciona la ruta completa
    std::string cmd = "python src/call_llm.py \"" + escaped_prompt + "\"";

    try {
        // Ejecutar el comando y capturar la respuesta
        std::string llm_response = execCommand(cmd);

        // Opcional: Puedes eliminar caracteres de nueva línea al final
        if (!llm_response.empty() && llm_response.back() == '\n') {
            llm_response.pop_back();
        }

        return llm_response;
    }
    catch(const std::exception& e) {
        // Manejar errores según sea necesario
        std::cerr << "Error al obtener la respuesta del LLM: " << e.what() << std::endl;
        return "";
    }
}
