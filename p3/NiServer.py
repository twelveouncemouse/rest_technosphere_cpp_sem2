import socket
import sys

NI_PORTS = 9000, 9001
PI_PORTS = 9100, 9101
CHUNK_SIZE = 16

class Server:
    def __init__(self, port, main_message):
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__socket_addr = ("127.0.0.1", port)
        self.__main_message = main_message
        self.__port = port

    def start(self):
        self.__socket.bind(self.__socket_addr)
        self.__socket.listen(2)
        print "Server at port {0} started".format(self.__socket_addr[1])
        while self.__socket is not None:
            print 'waiting for a connection'
            connection, client_address = self.__socket.accept()
            print "Connection from {0} accepted".format(client_address)
            self.__process_connection(connection, client_address)
        print "Server stopped"

    def __process_connection(self, connection, client_address):
        stop_flag = False
        self.__send_message(connection, "Greetings, {0}!\n".format(client_address))
        for msg in self.__recv_messages(connection):
            print "received: {0}".format(msg)
            if msg.startswith("Bye"):
                break
            elif msg.startswith("STOP"):
                stop_flag = True
                break
            elif msg.startswith("Who are you?"):
                self.__send_message(connection,
                                    "I'am {0} from servers who say \"{1}!\"\n".format(self.__port, self.__main_message))
            else:
                self.__send_message(connection, "{0}!\n".format(self.__main_message))
        connection.shutdown(socket.SHUT_RDWR)
        connection.close()
        if stop_flag:
            self.stop()

    @staticmethod
    def __recv_messages(conn):
        message = []
        while True:
            data = conn.recv(CHUNK_SIZE)
            if data:
                if "\n" in data:
                    idx = data.index("\n")
                    message += data[:idx]
                    yield "".join(message)
                    if idx != len(message) - 1:
                        message = data[idx + 1:]
                    else:
                        message = []
                else:
                    message += data
            else:
                break

    @staticmethod
    def __send_message(conn, msg):
        conn.sendall(msg)

    def stop(self):
        self.__socket.shutdown(socket.SHUT_RDWR)
        self.__socket.close()
        self.__socket = None


# servers = []
# for port in NI_PORTS:
#     server = Server(port, "Ni")
#     server.start()
#     servers.append(server)
# for port in PI_PORTS:
#     server = Server(port, "Pi")
#     server.start()
#     servers.append(server)

port, message = sys.argv[1:3]
serv = Server(port=int(port), main_message=message)
serv.start()