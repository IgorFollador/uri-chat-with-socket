import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class JavaServer {
    private static final int PORT = 8080;
    private static final int MAX_CLIENTS = 10;

    private final ServerSocket serverSocket;
    private final ExecutorService pool;
    private final List<ClientHandler> clients = Collections.synchronizedList(new ArrayList<>());

    public JavaServer() throws IOException {
        serverSocket = new ServerSocket(PORT);
        pool = Executors.newFixedThreadPool(MAX_CLIENTS);
    }

    public void start() {
        System.out.println("Servidor de chat está ouvindo na porta " + PORT + "...");
        while (true) {
            try {
                Socket clientSocket = serverSocket.accept();
                if (clients.size() < MAX_CLIENTS) {
                    ClientHandler clientHandler = new ClientHandler(clientSocket, this);
                    clients.add(clientHandler);
                    pool.execute(clientHandler);
                } else {
                    System.out.println("Número máximo de clientes atingido. Rejeitando nova conexão.");
                    clientSocket.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static void main(String[] args) throws IOException {
        new JavaServer().start();
    }

    private static class ClientHandler implements Runnable {
        private final Socket socket;
        private final JavaServer server;
        private PrintWriter out;
        private BufferedReader in;

        public ClientHandler(Socket socket, JavaServer server) {
            this.socket = socket;
            this.server = server;
        }

        @Override
        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);

                String message;
                while ((message = in.readLine()) != null) {
                    System.out.printf("Cliente %s: %s%n", socket.getRemoteSocketAddress(), message);
                    for (ClientHandler client : server.clients) {
                        if (client != this) {
                            client.out.printf("Cliente %s: %s%n", socket.getRemoteSocketAddress(), message);
                        }
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                server.clients.remove(this);
            }
        }
    }
}