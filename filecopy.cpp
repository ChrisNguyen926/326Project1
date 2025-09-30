#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <iostream>
#include <string>

#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[]) {
  char buffer[4096];
  int fd[2];
  pid_t pid;

  if (argc < 3) {
    std::cerr << "Enter: ./filecopy [source] [destination]" << std::endl;
    exit(1);
  }

  std::string src(argv[1]);
  std::string dst(argv[2]);

  // std::cout << "source file: " << src << "\n" << "destination file: " << dest << std::endl;
  if (pipe(fd) == -1) {
    perror("pipe failed");
    exit(1);
  }

  // set the fork return value to the process ID, 0 = child, 1 = parent
  pid = fork();

  if (pid < 0) {
    // fork fails

    perror("fork failed");
    exit(1);
  }
  else if (pid > 0) {
    // parent

    std::ifstream inFile(src);
    if (!inFile.is_open()) {
      std::cerr << "Failed to open source file" << std::endl;
      exit(1);
    }

    // close the read end as it is unused
    close(fd[READ_END]);

    // reads the contents of the file into the buffer, processing final bytes if needed
    // then writes the buffer into the pipe

    while (inFile.read(buffer, sizeof(buffer)) || inFile.gcount() > 0) {
      ssize_t n = inFile.gcount();
      write(fd[WRITE_END], buffer, inFile.gcount());
    }

    inFile.close();
    close(fd[WRITE_END]);
  }
  else {
    // Child

    std::ofstream outFile(dst);
    if (!outFile.is_open()) {
      exit(1);
    }

    // close the write end as it is unused
    close(fd[WRITE_END]);
    ssize_t n;

    // reads from the pipe and writes contents to the destination file
    while ((n = read(fd[READ_END], buffer, sizeof(buffer))) > 0) {
      outFile.write(buffer, n);
    }

    std::cout << "File successfully copied from " << src << " to " << dst << std::endl;

    outFile.close();
    close(fd[READ_END]);
  }

  return 0;
}