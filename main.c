#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

#define NORMAL_FILE 0x00
#define BMP_FILE    0x01
#define LINK_FILE   0x02
#define DIRECTORY   0x04

#define BUFFER_SIZE 4098

struct Files {
    char fileName[50];
    int height;
    int width;
    int size;
    int linkSize;
    int userId;
    int lastModifyDay;
    int lastModifyMonth;
    int lastModifyYear;
    int connectionsCounter;
    char userAccessRights[4];
    char groupAccessRights[4];
    char otherAccessRights[4];
}file;

int readFileProprieties(char *fileName, int fileType)
{
    struct stat stats;
    struct stat linkTarget;
    struct tm dt;
    int fisIn, rd;
    char buffer[BUFFER_SIZE];


    if((fisIn = open(fileName,O_RDONLY)) == -1)
    {
        printf("Can't open file:%s",fileName);
        return -1;
    }

    if(fileType == BMP_FILE)
    {
        if((rd = read(fisIn, buffer, BUFFER_SIZE)) > 0)
        {
            memcpy(&file.width, buffer + 18, 4);
            memcpy(&file.height, buffer + 22, 4);
        }
    }

    if (stat(fileName, &stats) == 0)
    {
        if(fileType == LINK_FILE)
        {
            if(lstat(fileName, &linkTarget) == 0)
            {
                file.linkSize = stats.st_size;
                file.size = linkTarget.st_size;
            }
        }
        else
        {
            file.userId = stats.st_uid;
        }

        if(fileType == NORMAL_FILE || fileType == BMP_FILE)
        {
            file.size = stats.st_size;

            dt = *(gmtime(&stats.st_mtime));
            file.lastModifyDay = dt.tm_mday;
            file.lastModifyMonth = dt.tm_mon;
            file.lastModifyYear = dt.tm_year + 1900;

            file.connectionsCounter = stats.st_nlink;
        }

        if (stats.st_mode & S_IRUSR){
            file.userAccessRights[0] = 'R';
        } else {
            file.userAccessRights[0] = '-';
        }

        if (stats.st_mode & S_IWUSR){
            file.userAccessRights[1] = 'W';
        } else {
            file.userAccessRights[1] = '-';
        }
        if (stats.st_mode & S_IXUSR){
            file.userAccessRights[2] = 'X';
        } else {
            file.userAccessRights[2] = '-';
        }

        if (stats.st_mode & S_IROTH){
            file.otherAccessRights[0] = 'R';
        } else {
            file.otherAccessRights[0] = '-';
        }

        if (stats.st_mode & S_IWOTH){
            file.otherAccessRights[1] = 'W';
        } else {
            file.otherAccessRights[1] = '-';
        }
        if (stats.st_mode & S_IXOTH){
            file.otherAccessRights[2] = 'X';
        } else {
            file.otherAccessRights[2] = '-';
        }

        if (stats.st_mode & S_IRGRP){
            file.groupAccessRights[0] = 'R';
        } else {
            file.groupAccessRights[0] = '-';
        }

        if (stats.st_mode & S_IWGRP){
            file.groupAccessRights[1] = 'W';
        } else {
            file.groupAccessRights[1] = '-';
        }
        if (stats.st_mode & S_IXGRP){
            file.groupAccessRights[2] = 'X';
        } else {
            file.groupAccessRights[2] = '-';
        }

        file.groupAccessRights[3] = '\0';
        file.userAccessRights[3] = '\0';
        file.otherAccessRights[3] = '\0';
    }
    else
    {
        printf("Unable to get file properties.\n");
        printf("Please check whether '%s' file exists.\n", fileName);
    }

    close(fisIn);
    return 0;
}

int writeFileProprieties(int fileType)
{
    int fisOut,chOut=0;
    char buffer[BUFFER_SIZE];

    if((fisOut = open("statistica.txt",O_CREAT | O_WRONLY | O_APPEND)) == -1)
    {
        printf("Can't open file:statistica.txt");
        return -1;
    }

    if(fileType == BMP_FILE)
    {
        chOut = sprintf(buffer,"nume fisier:%s\n"
                               "inaltime:%d\n"
                               "lungime:%d\n"
                               "dimensiune:%d\n"
                               "identificatorul utilizatorului:%d\n"
                               "timpul ultimei modificari:%d.%d.%d\n"
                               "contorul de legaturi:%d\n"
                               "drepturi de acces user:%s\n"
                               "drepturi de acces grup:%s\n"
                               "drepturi de acces altii:%s\n\n",
                        file.fileName,
                        file.height,
                        file.width,
                        file.size,
                        file.userId,
                        file.lastModifyDay,
                        file.lastModifyMonth,
                        file.lastModifyYear,
                        file.connectionsCounter,
                        file.userAccessRights,
                        file.groupAccessRights,
                        file.otherAccessRights);
    }
    else if(fileType == NORMAL_FILE)
    {
        chOut = sprintf(buffer,"nume fisier:%s\n"
                               "dimensiune:%d\n"
                               "identificatorul utilizatorului:%d\n"
                               "timpul ultimei modificari:%d.%d.%d\n"
                               "contorul de legaturi:%d\n"
                               "drepturi de acces user:%s\n"
                               "drepturi de acces grup:%s\n"
                               "drepturi de acces altii:%s\n\n",
                        file.fileName,
                        file.size,
                        file.userId,
                        file.lastModifyDay,
                        file.lastModifyMonth,
                        file.lastModifyYear,
                        file.connectionsCounter,
                        file.userAccessRights,
                        file.groupAccessRights,
                        file.otherAccessRights);
    }
    else if(fileType == LINK_FILE)
    {
        chOut = sprintf(buffer,"nume legatura:%s\n"
                               "dimensiune legatura:%d\n"
                               "dimensiunea fisierului target:%d\n"
                               "drepturi de acces user legatura:%s\n"
                               "drepturi de acces grup legatura:%s\n"
                               "drepturi de acces altii legatura:%s\n\n",
                        file.fileName,
                        file.linkSize,
                        file.size,
                        file.userAccessRights,
                        file.groupAccessRights,
                        file.otherAccessRights);
    }
    else if(fileType == DIRECTORY)
    {
        chOut = sprintf(buffer,"nume director:%s\n"
                               "identificatorul utilizatorului:%d\n"
                               "drepturi de acces user:%s\n"
                               "drepturi de acces grup:%s\n"
                               "drepturi de acces altii:%s\n\n",
                        file.fileName,
                        file.userId,
                        file.userAccessRights,
                        file.groupAccessRights,
                        file.otherAccessRights);
    }

    if(chOut != 0)
    {
        write(fisOut,buffer,chOut);
    }
    else
    {
        printf("No proprieties to get.\n");
    }


    close(fisOut);
    return 0;
}

int getFileProprieties(char *folderName)
{
    struct dirent *files;
    struct stat stats;
    DIR *folder;
    char filePath[BUFFER_SIZE];
    const char *ext;

    if((folder = opendir(folderName)) == NULL)
    {
        printf("Can't open folder\n");
        return -1;
    }

    while((files = readdir(folder)) != NULL)
    {
        sprintf(filePath,"%s/%s",folderName,files->d_name);
        if((stat(filePath, &stats) == 0))
        {
            if(S_ISREG(stats.st_mode))
            {
                strcpy(file.fileName,files->d_name);
                ext = strchr (filePath, '.');
                if(((ext!= NULL) && (!strcmp(ext+1,"bmp"))))
                {

                    readFileProprieties(filePath,BMP_FILE);
                    writeFileProprieties(BMP_FILE);
                }
                else
                {
                    readFileProprieties(filePath,NORMAL_FILE);
                    writeFileProprieties(NORMAL_FILE);
                }
            }
            else if(S_ISLNK(stats.st_mode))
            {
                strcpy(file.fileName,files->d_name);
                readFileProprieties(filePath,LINK_FILE);
                writeFileProprieties(LINK_FILE);
            }
            else if(S_ISDIR(stats.st_mode))
            {
                if(strcmp(files->d_name,".") != 0 && strcmp(files->d_name,"..") != 0)
                {
                    strcpy(file.fileName,files->d_name);
                    readFileProprieties(filePath,DIRECTORY);
                    writeFileProprieties(DIRECTORY);
                }
            }
            else
            {
                printf("No proprieties will be shown.\n");
            }
        }
    }

    closedir(folder);
    return 0;
}

int main(int argv, char **argc)
{
    if(argv != 2){
        printf("Error number of arguments");
        return -1;
    }
    getFileProprieties(argc[1]);
    return 0;
}
