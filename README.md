mqsocket
========

Extended mqsocket.dll functionalities based on http://www.mql5.com/en/code/169


Added functions:


uint SocketReadString(
                       SOCKET_CLIENT &cl,// variable to serve connection data
                       string &buffer,       // string as buffer for reply
                       int   timeout_ms     // Time to wait for sock recv before giving up
                       );

uint SocketWriteString_ExpectReply(
                       SOCKET_CLIENT &cl,   // variable to serve connection data
                       string str,          // string to send
                       string &reply,       // string as buffer for reply
                       int   timeout_ms     // Time to wait for sock recv before giving up
                       );


