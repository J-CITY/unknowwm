#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <map>

namespace ap {

enum class PayloadType {
    NONE = 0,
    TYPE_WSTRING = 1,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL
};

struct Payload {
private:
    PayloadType type = PayloadType::NONE;
public:
    Payload() {}
    Payload(std::wstring wstrVal): wstrVal(wstrVal) {}
    Payload(int intVal) : intVal(intVal) {}
    Payload(float floatVal) : floatVal(floatVal) {}
    Payload(bool boolVal) : boolVal(boolVal) {}
    PayloadType getType() {return type;}

    void set(std::wstring wstrVal) { this->wstrVal = wstrVal; type = PayloadType::TYPE_WSTRING; }
    void set(int intVal) { this->intVal = intVal; type = PayloadType::TYPE_INT; }
    void set(float floatVal) { this->floatVal = floatVal; type = PayloadType::TYPE_FLOAT; }
    void set(bool boolVal) { this->boolVal = boolVal;  type = PayloadType::TYPE_BOOL; }

    int intVal = -1;
    std::wstring wstrVal = L"";
    float floatVal = 0.0f;
    bool boolVal = false;
};

class ArgParse {
public:
    ArgParse(std::wstring programDescription=L"") {
        allDescription = programDescription + L"\n\n";

        //add -h --help info key
        allDescription += L"--h -help : print information about program keys\n";
    }

    ~ArgParse() {

    }
    std::vector<std::vector<std::wstring>> keysData;

    std::map<std::wstring, std::function<void()>> argsMap;
    std::map<std::wstring, PayloadType> argsDataType;
    std::map<std::wstring, std::shared_ptr<Payload>> payloadMap;

    void addArg(std::vector<std::wstring> keys,
                PayloadType dataType, std::function<void()> fun,
                Payload* defPayload=nullptr, std::wstring description = L"") {
        std::shared_ptr<Payload> p(defPayload == nullptr ? new Payload() : defPayload);
        for (auto& k : keys) {
            argsMap[k] = fun;
            argsDataType[k] = dataType;
            allDescription += k + L" - ";
            payloadMap[k] = p;
        }

        allDescription += description + L"\n";
        keysData.push_back(keys);
    }

    void parse(int argc, char* argv[], bool needRun=true) {
        if (argc <= 1) {
            return;
        }
        auto i = 1;
        while (i < argc) {
            std::string str(argv[i]);
            std::wstring cmd(str.begin(), str.end());

            auto p = getPayload(cmd);

            if (cmd == L"-h" || cmd == L"--help") {
                std::wcout << allDescription << std::endl;
            }

            else if (argsDataType[cmd] == PayloadType::TYPE_WSTRING) {
                i++;
                if (i < argc) {
                    std::string _str(argv[i]);
                    p->set(std::wstring(_str.begin(), _str.end()));
                } else {
                    std::cerr << "Err: bad string param";
                }
            }
            else if (argsDataType[cmd] == PayloadType::TYPE_INT) {
                i++;
                if (i < argc) {
                    p->set(std::stoi(argv[i]));
                } else {
                    std::cerr << "Err: bad int param";
                }
            }
            else if (argsDataType[cmd] == PayloadType::TYPE_FLOAT) {
                i++;
                if (i < argc) {
                    p->set(std::stof(argv[i]));
                } else {
                    std::cerr << "Err: bad float param";
                }
            }
            else if (argsDataType[cmd] == PayloadType::TYPE_BOOL) {
                p->set(!p->boolVal);
            }

            if (needRun) {
                run(cmd);
            }
            i++;
        }
    }

    std::shared_ptr<Payload> getPayload(std::wstring& key) {
        if (payloadMap.find(key) == payloadMap.end()) {
            return nullptr;
        }
        return payloadMap[key];
    }

    void run(std::wstring key) {
        auto res = argsMap.find(key);
        if (res != argsMap.end()) {
            argsMap[key]();
        }
    }

private:
    std::wstring allDescription = L"";

};
};