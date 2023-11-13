#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define BUFFER_SIZE 285
 struct Images {
    char imgName[50];
    int height;
    int width;
    int size;
    int userId;
    int lastModifyDay;
    int lastModifyMonth;
    int lastModifyYear;
    int connectionsCounter;
    char userAccessRights[4];
    char groupAccessRights[4];
    char otherAccessRights[4];
}image;

int readFileProprieties(char *fileName)
{
    struct stat stats;
    struct tm dt;
    int fisIn, rd;
    char buffer[BUFFER_SIZE];
    const char *ext = strchr (fileName, '.');
    if(!((ext!= NULL) && (!strcmp(ext+1,"bmp")))){
        printf("You have read a different file, not a bmp");
        return -1;
    }
    if((fisIn = open(fileName,O_RDONLY)) == -1)
    {
        printf("Can't open file:%s",fileName);
        return -1;
    }

    strcpy(image.imgName,fileName);

    if((rd = read(fisIn, buffer, BUFFER_SIZE)) > 0)
    {
        memcpy(&image.width, buffer + 18, 4);
        memcpy(&image.height, buffer + 22, 4);
    }
    if (stat(fileName, &stats) == 0)
    {
        if (stats.st_mode & S_IRUSR){
            image.userAccessRights[0] = 'R';
        } else {
            image.userAccessRights[0] = '-';
        }

        if (stats.st_mode & S_IWUSR){
            image.userAccessRights[1] = 'W';
        } else {
            image.userAccessRights[1] = '-';
        }
        if (stats.st_mode & S_IXUSR){
            image.userAccessRights[2] = 'X';
        } else {
            image.userAccessRights[2] = '-';
        }

        if (stats.st_mode & S_IROTH){
            image.otherAccessRights[0] = 'R';
        } else {
            image.otherAccessRights[0] = '-';
        }

        if (stats.st_mode & S_IWOTH){
            image.otherAccessRights[1] = 'W';
        } else {
            image.otherAccessRights[1] = '-';
        }
        if (stats.st_mode & S_IXOTH){
            image.otherAccessRights[2] = 'X';
        } else {
            image.otherAccessRights[2] = '-';
        }

        if (stats.st_mode & S_IRGRP){
            image.groupAccessRights[0] = 'R';
        } else {
            image.groupAccessRights[0] = '-';
        }

        if (stats.st_mode & S_IWGRP){
            image.groupAccessRights[1] = 'W';
        } else {
            image.groupAccessRights[1] = '-';
        }
        if (stats.st_mode & S_IXGRP){
            image.groupAccessRights[2] = 'X';
        } else {
            image.groupAccessRights[2] = '-';
        }

        image.groupAccessRights[3] = '\0';
        image.userAccessRights[3] = '\0';
        image.otherAccessRights[3] = '\0';

        image.size = stats.st_size;

        dt = *(gmtime(&stats.st_mtime));
        image.lastModifyDay = dt.tm_mday;
        image.lastModifyMonth = dt.tm_mon;
        image.lastModifyYear = dt.tm_year + 1900;

        image.connectionsCounter = stats.st_nlink;

        image.userId = stats.st_uid;
    }
    else
    {
        printf("Unable to get file properties.\n");
        printf("Please check whether '%s' file exists.\n", fileName);
    }

    close(fisIn);
    return 0;
}

int writeProprieties()
{
    int fisOut;
    char buffer[BUFFER_SIZE];
    if((fisOut = open("C:\\Users\\cosmi\\CLionProjects\\so-proiect\\statistica.txt",O_CREAT | O_WRONLY | O_TRUNC)) == -1)
    {
        printf("Can't open file:statistica.txt");
        return -1;
    }

    sprintf(buffer,"nume fisier:%s\n"
                   "inaltime:%d\n"
                   "lungime:%d\n"
                   "dimensiune:%d\n"
                   "identificatorul utilizatorului:%d\n"
                   "timpul ultimei modificari:%d.%d.%d\n"
                   "contorul de legaturi:%d\n"
                   "drepturi de acces user:%s\n"
                   "drepturi de acces grup:%s\n"
                   "drepturi de acces altii:%s\n",
                   image.imgName,
                   image.height,
                   image.width,
                   image.size,
                   image.userId,
                   image.lastModifyDay,
                   image.lastModifyMonth,
                   image.lastModifyYear,
                   image.connectionsCounter,
                   image.userAccessRights,
                   image.groupAccessRights,
                   image.otherAccessRights);
    buffer[strlen(buffer)] = '\0';
    printf("%d\n",write(fisOut,buffer,BUFFER_SIZE));
    close(fisOut);
    return 0;
}

int main(int argv, char **argc)
{
    if(argv != 2){
        printf("Error number of arguments");
        return -1;
    }
    readFileProprieties(argc[1]);
    writeProprieties();
    return 0;
}
