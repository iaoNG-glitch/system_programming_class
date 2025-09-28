
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1
#define EXT ".txt"

// .txt 확장자인지 확인하는 함수
int has_txt_extension(const char *filename) {
    size_t len = strlen(filename);
    size_t ext_len = strlen(EXT);
    if (len < ext_len) return 0;
    return strcmp(filename + len - ext_len, EXT) == 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <source_directory> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *source_dir = argv[1];
    const char *output_file = argv[2];

    DIR *dir = opendir(source_dir);
    if (dir == NULL) {
        perror("디렉토리를 열 수 없습니다");
        exit(EXIT_FAILURE);
    }

    int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) {
        perror("출력 파일을 열 수 없습니다");
        closedir(dir);
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        // "." 또는 ".." 무시
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // 파일 경로 생성 (상대 경로)
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", source_dir, entry->d_name);

        // stat()으로 파일 타입 확인
        struct stat file_stat;
        if (stat(filepath, &file_stat) == -1) {
            perror("파일 정보 확인 실패");
            continue;
        }

        // 일반 파일이 아니면 건너뜀
        if (!S_ISREG(file_stat.st_mode)) continue;

        // .txt 파일이 아니면 건너뜀
        if (!has_txt_extension(entry->d_name)) continue;

        // txt 파일 열기
        int in_fd = open(filepath, O_RDONLY);
        if (in_fd == -1) {
            perror("입력 파일을 열 수 없습니다");
            continue;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while ((bytes_read = read(in_fd, buffer, BUFFER_SIZE)) > 0) {
            if (buffer[0] != ' ' && buffer[0] != '\n' && buffer[0] != '\t') {
                if (write(out_fd, buffer, bytes_read) == -1) {
                    perror("쓰기 오류");
                    close(in_fd);
                    close(out_fd);
                    closedir(dir);
                    exit(EXIT_FAILURE);
                }
            }
        }

        close(in_fd);
    }

    close(out_fd);
    closedir(dir);

    return 0;
}
