mqsocket
========

Extended mqsocket.dll functionalities based on http://www.mql5.com/en/code/169


Added functions:


    uint SocketReadString(
                       SOCKET_CLIENT &cl,    
                       string &buffer,       
                       int   timeout_ms     
                       );

    uint SocketWriteString_ExpectReply(
                       SOCKET_CLIENT &cl,   
                       string str,         
                       string &reply,       
                       int   timeout_ms     
                       );


