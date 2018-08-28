/*
Pipe.h
Written by Matthew Fisher

A pipe is a connection between two programs, possibly on different computers.
*/
#include <string>
 
class Pipe
{
public:
    Pipe();
    ~Pipe();
    //
    // Connection
    //
    void ClosePipe();
    void CreatePipe(const std::string &PipeName);
    void ConnectToLocalPipe(const std::string &PipeName);
    void ConnectToPipe(const std::string &PipeName);

    //
    // Messaging
    //
    bool MessagePresent();
    bool ReadMessage(std::string &strRead);
    void PipeSendMessage(const BYTE *Message, UINT MessageLength);
	void PipeSendMessage(const std::string &Message);

    //
    // Query
    //
    UINT ActiveInstances();
    std::string UserName();
    __forceinline bool Valid()
    {
        return (_Handle != NULL);
    }
    
private:
    HANDLE _Handle;
};
