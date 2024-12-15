#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>

std::string pipeRead(int pipe)
{
    std::string message;
    char buffer[128];  // Tymczasowy bufor do przechowywania danych z potoku
    ssize_t bytesRead;

    // Odczytujemy dane z potoku i zapisujemy je do zmiennej message
    while ((bytesRead = read(pipe, buffer, sizeof(buffer))) > 0) 
    {
        message.append(buffer, bytesRead);
    }
    
    if (bytesRead == -1)
    {
        perror("Error reading from pipe");
        exit(EXIT_FAILURE);
    }

    return message;
}

void pipeWrite(int pipe, std::string message)
{
    // Zapisujemy tekst z parametru message do potoku
    ssize_t bytesWritten = write(pipe, message.c_str(), message.size());
    
    if (bytesWritten == -1)
    {
        perror("Error writing to pipe");
        exit(EXIT_FAILURE);
    }
}

std::vector<std::array<int, 2>> createPipes(int count)
{
    std::vector<std::array<int, 2>> pipes;
    for (int i = 0; i < count; ++i)
    {
        std::array<int, 2> newPipe;
        if (pipe(newPipe.data()) == -1)
        {
            perror("Error creating pipe.");
            exit(EXIT_FAILURE);
        }
        pipes.push_back(newPipe);
    }
    return pipes;
}


int main(int argc, char* argv[])
{
    auto pipes = createPipes(1);  // Tworzymy jeden potok

    if (fork() == 0)
    {
        // Proces potomny
        close(pipes[0][0]);  // Zamykamy koniec potoku, z którego będziemy pisać
        std::string message = "komunikat przesłany przez potok";
        pipeWrite(pipes[0][1], message);  // Zapisujemy komunikat do potoku
        close(pipes[0][1]);  // Zamykamy koniec zapisu
    }
    else
    {
        // Proces macierzysty
        close(pipes[0][1]);  // Zamykamy koniec potoku, z którego będziemy czytać

        // Odczytujemy dane z potoku
        std::string message = pipeRead(pipes[0][0]);
        std::cout << "Otrzymano wiadomość: " << message << "\n";  // Wyświetlamy komunikat
        close(pipes[0][0]);  // Zamykamy koniec odczytu

        wait(nullptr);  // Czekamy na zakończenie procesu potomnego
    }

    return 0;
}
