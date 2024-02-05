#include <stdio.h>
#include <string.h>


void setZero(char *m, int size) {
    for (int i = 0; i < size; ++i) {
        m[i] = '\0';
    }
} ///заполняет нулями
void RandW(char *s1, char *s2, int sizeS2, char *mass) {
    //переводит указатель с первого символа на символ после =
    char *ch = strstr(s1, s2) + sizeS2;
    int ind = 0;
    while ((*ch != ' ') && (*ch != '\0')) {
        mass[ind++] = *ch++;
    }
} ///считывает значение параметра (то, что после =)
typedef struct {
    char id[3];
    char version[2];
    char flags[1];
    char size[4];
} ID3v2; ///хэдер
typedef struct {
    char id[4];
    char size[4];
    char flags[2];
} frame; ///фрейм
int getSize(const char *size) {
    return (size[0] << 21) | (size[1] << 14) | (size[2] << 7) | size[3];
} ///возвращает размер значения в байтах
void ShowFrames(FILE *f) {
    fseek(f, 0, SEEK_SET);
    ID3v2 header;
    fread(&header, sizeof(header), 1, f);


    frame frame;
    unsigned long long count = 10;

    while (count < getSize(header.size)) {
        fread(&frame, sizeof(frame), 1, f);
        count = count + sizeof(frame) + getSize(frame.size);
        if (getSize(frame.size) != 0){
            printf("Frame: %s\n", frame.id);
            printf("Value: ");
            for (int i = 0; i < getSize(frame.size); ++i) {
                printf("%c", getc(f));
            }
            printf("\n\n");
        }
    }
    printf("\n\n");
} ///показывает все метаданные файла
void FindFrame(FILE *f, char *par) {
    ID3v2 header;
    fread(&header, sizeof(header), 1, f);

    unsigned long long count = 10; //первые 10 байт уже считаны (header)
    fseek(f, 0, SEEK_SET);
    frame frame;


    while (count < getSize(header.size)) {
        fread(&frame, sizeof(frame), 1, f);
        count += sizeof(frame) + getSize(frame.size);

        if (strstr(frame.id, par) != NULL) {
            printf("%s: ", frame.id);
            for (int i = 0; i < getSize(frame.size); ++i) {
                printf("%c", getc(f));
            }
            printf("\n");
            return;
        }
        for (int i = 0; i < getSize(frame.size); ++i){
            getc(f);
        }
    }
    printf("Frame %s is empty!\n", par);
} ///показывает значение конкретного тега метаданных
void ReplaceFrame(FILE *f, char *path, char *par, char *replacement) {
    unsigned long long count = 10; //первые 10 байт уже считаны
    fseek(f, 0, SEEK_SET);
    ID3v2 header;
    frame frame;
    fread(&header, sizeof(header), 1, f);

    ///создаём новый файл для записи и записываем header
    FILE *f2 = fopen("Song2.mp3", "wb");
    fwrite(&header, sizeof(header), 1, f2);


    ///записываем метаданные
    while(count < getSize(header.size)){
        fread(&frame, sizeof(frame), 1, f); //считываем

        ///если находим нужный тег
        if (strstr(frame.id, par) != NULL) {
            fwrite(&frame, sizeof(frame), 1, f2); //записываем

            ///записываем в f2 новое значения тега
            for (int i = 0; i < strlen(replacement); ++i){
                putc(replacement[i], f2);
            }
            ///пропускаем тег в f
            for (int i = 0; i < getSize(frame.size); ++i) {
                getc(f);
            }
        }
        else {
            fwrite(&frame, sizeof(frame), 1, f2); //записываем
            for (int i = 0; i < getSize(frame.size); ++i) {
                putc(getc(f), f2);
            }
        }
        count += sizeof(frame) + getSize(frame.size); //прибавляем к счётчику
    }



    ///теперь дописываем сам аудиофайл
    char buffer;
    while (fread(&buffer, 1, 1, f) > 0){
        putc(buffer, f2);
    }

    ///закрываем все файлы
    fclose(f);
    fclose(f2);

    ///удаляем старый
    remove(path);

    ///переименовываем новый
    rename("Song2.mp3", path);

} ///изменяет значение тега

int main(int argc, char **argv) {
    //printf()
    ///Лабораторная работа №13
    char Parameter[1000], filepath[1000], Value[1000];
    ///заполняем всё нулями
    setZero(Parameter, 1000);
    setZero(filepath, 1000);
    setZero(Value, 1000);


    ///строка, в которой ищем; строка, которую ищем; её длина; массив для записи
    RandW(argv[1], "--filepath=", 11, filepath);
    ///для чтения и записи в бинарном режиме
    FILE *f = fopen(filepath, "rb");




    if (strstr(argv[2], "--show") != NULL) {
        ShowFrames(f);
    } else if ((strstr(argv[2], "--set=") != NULL) && (strstr(argv[3], "--value=") != NULL)) {
        RandW(argv[2], "--set=", 6, Parameter);
        RandW(argv[3], "--value=", 8, Value);
        ReplaceFrame(f, filepath, Parameter, Value);
    } else if (strstr(argv[2], "--get=") != NULL) {
        RandW(argv[2], "--get=", 6, Parameter);
        FindFrame(f, Parameter);
    } else {
        printf("You didn't enter anything!");
    }

    fclose(f);
    return 0;
}
//main.exe --filepath=Song.mp3 --set=TIT2 --value=ABC
//main.exe --filepath=Song.mp3 --show
//main.exe --filepath=Song.mp3 --get=TIT2