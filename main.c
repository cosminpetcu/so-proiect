#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

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

int convertToGrayScale(const char *inputFileName)
{
    int fisIn;
    unsigned char buffer[3];
    unsigned char red,green,blue,gray;

    if((fisIn = open(inputFileName,O_RDWR)) == -1)
    {
        perror("Can't open input file");
        return -1;
    }

    lseek(fisIn,54,SEEK_SET);

    while(read(fisIn,buffer,sizeof(buffer)) > 0)
    {
        red = buffer[2];
        green = buffer[1];
        blue = buffer[0];

        gray = (unsigned char)(0.299 * red + 0.587 * green + 0.114 * blue);

        lseek(fisIn,-3,SEEK_CUR);

        write(fisIn,&gray,sizeof(gray));

        lseek(fisIn,2,SEEK_CUR);
    }

    close(fisIn);
    return 0;
}

int readFileProprieties(const char *fileName, int fileType)
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

int writeFileProprieties(const char *outputFolder,int fileType, int *linesWritten)
{
    int fisOut,chOut = 0;
    char buffer[BUFFER_SIZE];
    char outputFilePath[BUFFER_SIZE];

    snprintf(outputFilePath, sizeof(outputFilePath), "%s/%s_statistica.txt", outputFolder, file.fileName);

    if((fisOut = open(outputFilePath,  O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
    {
        printf("Can't open file:%s",outputFilePath);
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
        *linesWritten = 10;
    }
    else if(fileType == NORMAL_FILE)
    {
        chOut = sprintf(buffer,"nume fisier:%s\n"
                   "dimensiune:%d\n"
                   "Identificatorul utilizatorului:%d\n"
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
        *linesWritten = 8;
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
        *linesWritten = 6;
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
        *linesWritten = 5;
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
    return chOut;
}

int readWriteProprieties(const char *filePath, int fileType, const char *outputFolder, const char *character)
{
    pid_t pid,pid2;
    int status,status2,statistica,linesWritten = 0,chOut = 0, fileSize = 0, readFile = 0;
    char buffer[BUFFER_SIZE], fileContent[BUFFER_SIZE];
    int pipe_ff[2],pipe_fp[2];
    char outputFilePath[BUFFER_SIZE], buffer2[2];

    snprintf(outputFilePath, sizeof(outputFilePath), "%s/%s_statistica.txt", outputFolder, file.fileName);

    if((statistica = open("statistica.txt",  O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
    {
        printf("Can't open file:%s","statistica.txt");
        return -1;
    }

    if(fileType == NORMAL_FILE)
    {
        if (pipe(pipe_ff) == -1) 
        {
            perror("Pipe creation error");
            exit(-1);
        }
        if (pipe(pipe_fp) == -1) 
        {
            perror("Pipe creation error");
            exit(-1);
        }
    }

    if((pid = fork()) < 0)
    {
        perror("Fork error");
        exit(-1);
    }

    if(pid == 0)
    {
        if(fileType == NORMAL_FILE)
        {
            close(pipe_fp[0]);
            close(pipe_fp[1]);
            close(pipe_ff[0]);
        }

        readFileProprieties(filePath,fileType);
        fileSize = writeFileProprieties(outputFolder, fileType, &linesWritten);
        

        chOut = sprintf(buffer, "%d\n", linesWritten);
        write(statistica, buffer, chOut);

        if(fileType == BMP_FILE)
        {
            if((pid2 = fork()) < 0)
            {
                perror("Fork error");
                exit(-1);
            }

            if(pid2 == 0)
            {
                convertToGrayScale(filePath);
                exit(0);
            }
            else
            {
                waitpid(pid2, &status2, 0);
                if (WIFEXITED(status2))
                {
                    printf("Child process %d exited with status %d\n", pid2, WEXITSTATUS(status2));
                }
                else
                {
                    printf("Child process %d did not exit normally\n", pid2);
                }
            }
        }

        if(fileType == NORMAL_FILE)
        {
            if((readFile = open(outputFilePath,  O_RDONLY)) == -1)
            {
                printf("Can't open file:%s",outputFilePath);
                return -1;
            }
            
            read(readFile, fileContent, fileSize);
            write(pipe_ff[1], fileContent, fileSize);
        }

        close(pipe_ff[1]);
        exit(0);
    }
    else
    {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            printf("Child process %d exited with status %d\n", pid, WEXITSTATUS(status));
        }
        else
        {
            printf("Child process %d did not exit normally\n", pid);
        }
    }
    
    if(fileType == NORMAL_FILE)
    {
        if ((pid = fork()) < 0)
        {
            perror("Fork error");
            exit(-1);
        }
        if (pid == 0)
        {
            close(pipe_fp[0]);
            close(pipe_ff[1]);

            dup2(pipe_ff[0],0);
            dup2(pipe_fp[1],1);

            close(pipe_fp[1]);
            close(pipe_ff[0]);
                        
            execlp("bash", "bash", "script.sh", character, (char *)NULL);

            perror("Execution error");
            exit(-1);  
        }

        close(pipe_fp[1]);
        close(pipe_ff[0]);
        close(pipe_ff[1]);

        read(pipe_fp[0], buffer2, sizeof(char));
        buffer2[1] = '\0';
        printf("Au fost identificate in total %s propozitii corecte care contin caracterul %s\n", buffer2, character);

        close(pipe_fp[0]);
        close(readFile);
    }

    close(statistica);
    return 0;
}

int getFileProprieties(const char *inputFolder,const char *outputFolder, const char *character)
{
    struct dirent *files;
    struct stat stats;
    DIR *folder;
    char filePath[BUFFER_SIZE];
    const char *ext;

    if((folder = opendir(inputFolder)) == NULL)
    {
        printf("Can't open folder\n");
        return -1;
    }

    while((files = readdir(folder)) != NULL)
    {
        sprintf(filePath,"%s/%s",inputFolder,files->d_name);
        if((stat(filePath, &stats) == 0))
        {
            if(S_ISREG(stats.st_mode))
            {
                strcpy(file.fileName,files->d_name);
                ext = strchr (filePath, '.');
                if(((ext!= NULL) && (!strcmp(ext+1,"bmp"))))
                {
                    
                    readWriteProprieties(filePath,BMP_FILE,outputFolder,character);
                }
                else
                {
                    readWriteProprieties(filePath,NORMAL_FILE,outputFolder,character);
                }
            }
            else if(S_ISLNK(stats.st_mode))
            {
                strcpy(file.fileName,files->d_name);
                readWriteProprieties(filePath,LINK_FILE,outputFolder,character);
            }
            else if(S_ISDIR(stats.st_mode))
            {
                if(strcmp(files->d_name,".") != 0 && strcmp(files->d_name,"..") != 0)
                {
                    strcpy(file.fileName,files->d_name);
                    readWriteProprieties(filePath,DIRECTORY,outputFolder,character);
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
     if (argv != 4)
    {
        printf("Error number of arguments");
        return -1;
    }

    getFileProprieties(argc[1], argc[2], argc[3]);

    return 0;
}
