import java.io.*;
import java.net.*;

public class JavaClient {
    private static PrintWriter out;
    private static BufferedReader in;
    private static boolean keepRunning = true;

    public static void main(String[] args) throws IOException {
        // Configuração
        String serverAddress = "localhost";
        int serverPort = 8080;

        // Criação do socket e estabelecimento da conexão
        Socket socket = new Socket(serverAddress, serverPort);
        out = new PrintWriter(socket.getOutputStream(), true);
        in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        // Thread para ler as respostas do servidor
        Thread readThread = new Thread(() -> {
            try {
                String response;
                while (keepRunning && (response = in.readLine()) != null) {
                    System.out.println(response);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        });

        readThread.start();

        // Ler a entrada do usuário e enviar para o servidor
        System.out.println("Digite uma mensagem (ou 'sair' para encerrar):");
        BufferedReader userInput = new BufferedReader(new InputStreamReader(System.in));
        String message;

        while (true) {
            message = userInput.readLine();
            if ("sair".equalsIgnoreCase(message)) {
                keepRunning = false; // Encerra a thread de leitura
                break;
            }
            out.println(message);
            out.flush();
        }
        
        try {
            readThread.join(); // Espera a thread de leitura finalizar
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        socket.close();
    }
}