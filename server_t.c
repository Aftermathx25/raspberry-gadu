#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT_NUMBER 1994

struct klient {
	int gniazdo_k;
	struct sockaddr_in adres_k;
};

struct online {
	struct klient* id;
	char login[6];
};


char powitanie[] = "Witamy! Połączyłeś się z Raspberry Pi! Podaj login:\n\0";

void* odczyt(void* arg) {
	struct online* g = (struct online*)arg;
	struct klient* k = g->id;
	char login[6];
	strcpy(login, g->login);
	struct stat st;
	stat(login, &st); // Sprawdzanie początkowego rozmiaru pliku
	int size = st.st_size;
	int size2 = size+1;
	int fp;
	int i=0;
		while(1>0)
		{
			if(send(k->gniazdo_k, "", 0, MSG_NOSIGNAL)==-1) // Sprawdzanie statusu połączenia
				break;
			if(size2!=size)
			{
				i=0;
				printf("Przesyłanie wiadomości dla:%s\n", login);
				fp=open(login, O_RDONLY);
				if(fp==-1)
					printf("Błąd odczytu pliku w odczyt()");
				flock(fp, LOCK_EX); // Wejście do sekcji krytycznej
				printf("Uzyskano dostęp do pliku:%s przez proces odczyt()\n", login);	
				char bufor_tekst[256];
				size_t len = 0;
				while(read(fp, bufor_tekst, 255)!= 0)
				{	
					write(k->gniazdo_k, bufor_tekst, strlen(bufor_tekst)); //ZABEZPIECZYĆ
					bzero(bufor_tekst, 256);
				}
				bzero(bufor_tekst,256);
				close(fp);
				fp=open(login, O_TRUNC);
				flock(fp, LOCK_UN); // Wyjście z sekcji krytycznej
				close(fp);
				size2=0;
				size=size2;
			}
		stat(login, &st); // Sprawdzanie rozmiaru pliku
		size2 = st.st_size;
		}
	printf("Zakończono wątek odczyt()\n");
}


void* watek_klienta(void* arg) {
	pthread_t tid2;
	struct klient* k = (struct klient*)arg;
	if(write(k->gniazdo_k,powitanie,strlen(powitanie))==-1)
	{
		printf("Błąd zapisu na gniazdo dla IP:%s. Zamykam połączenie\n",inet_ntoa((struct in_addr)k->adres_k.sin_addr));
		close(k->gniazdo_k);
		free(k);
		return 0;	
	}
	char napis[256];
	bzero(napis,256);
	int n,found,username;
	FILE *fp;
	char linia[32];
	char login[6];
	bzero(login, 6);
	found = 0;
	username = 0;

	fp = fopen("login", "r");
	if (read(k->gniazdo_k,napis,255) == -1)
	{	
		printf("Błąd odczytu z gniazda dla IP:%s. Zamykam połączenie\n",inet_ntoa((struct in_addr)k->adres_k.sin_addr));
		close(k->gniazdo_k);
		free(k);	
		return 0;
	}
	while(fscanf(fp, "%s", linia)!=EOF) // Przeszukiwanie całego pliku
	{
		if(strncmp(linia, napis, strlen(napis)) == 0) // Jeżeli login pasuje
		{
			strncpy(login, napis, 6);
			char komenda[64];
			strcpy(komenda, "Login jest prawidłowy\n");
			username=1;
			if(write(k->gniazdo_k, komenda, strlen(komenda))==-1)
			{	
				printf("Błąd zapisu na gniazdo dla IP:%s. Zamykam połączenie\n",inet_ntoa((struct in_addr)k->adres_k.sin_addr));
				close(k->gniazdo_k);
				free(k);	
				return 0;
			}
			bzero(napis, strlen(napis));
			if(read(k->gniazdo_k, napis, 255)==-1) 
			{	
				printf("Błąd odczytu z gniazda dla IP:%s. Zamykam połączenie\n",inet_ntoa((struct in_addr)k->adres_k.sin_addr));
				close(k->gniazdo_k);
				free(k);	
				return 0;
			}
			bzero(komenda, strlen(komenda));
			strncpy(komenda, linia + 7, strlen(linia) - 7);
			
			if(strncmp(komenda, napis, strlen(napis))==0)
			{
				bzero(napis, strlen(napis));
				strcpy(napis, "Zalogowano pomyślnie.\n");
				if(write(k->gniazdo_k, napis, strlen(napis))==-1)
				{	
					printf("Błąd zapisu na gniazdo dla IP:%s. Zamykam połączenie\n",inet_ntoa((struct in_addr)k->adres_k.sin_addr));
					close(k->gniazdo_k);
					free(k);	
					return 0;
				}
				found = 1;
				break;
			}
		}
		
	}
	fclose(fp);
	if(found == 1)	
	{	
		printf("Zalogowano klienta:%s z IP:%s\n",login, inet_ntoa((struct in_addr)k->adres_k.sin_addr));
		struct online* g = malloc(sizeof(struct online));
		memcpy((void *)&g->id, (void *)&k, sizeof(k));
		memcpy((void *)&g->login, (void *)&login, sizeof(login));
		pthread_create(&tid2, NULL, odczyt, g);
		char bufor_odczytu[256];
		char bufor_odczytu2[256];
		char bufor_odczytu3[256];
		char numer_klienta[6];
		int fd;
		while(1>0)
		{
			if(send(k->gniazdo_k, "", 0, MSG_NOSIGNAL)==-1)
			{
				printf("Zerwano połączenie z klientem: %s\n", inet_ntoa((struct in_addr)k->adres_k.sin_addr));
				close(k->gniazdo_k);
				break;
			}
		     	if(read(k->gniazdo_k, bufor_odczytu2, 255)>8)
			{
				strncpy(numer_klienta, bufor_odczytu2, 6);
				strncpy(bufor_odczytu3, bufor_odczytu2+7, strlen(bufor_odczytu2)-7);
				sprintf(bufor_odczytu, "%s:%s", login, bufor_odczytu3);
				fd=open(numer_klienta, O_WRONLY);
				if(fd==-1)
				{
					printf("Bład otwarcia pliku w watek_klienta()\n");
					if(write(k->gniazdo_k, "blad1\n\0", 7)==-1)
					{
					printf("Błąd zapisu na gniazdo dla IP:%s. Zamykam połączenie\n",inet_ntoa((struct in_addr)k->adres_k.sin_addr));
					close(k->gniazdo_k);
					free(k);	
					return 0;	
					}
				}
				flock(fd, LOCK_EX);
				if(fd!=0)
				{
					lseek(fd, 0, SEEK_END);
					if(write(fd, bufor_odczytu, strlen(bufor_odczytu))==-1)
						printf("Błąd zapisu do pliku\n");
				}
				flock(fd, LOCK_UN);
				close(fd);
			        bzero(bufor_odczytu, 256);
				bzero(bufor_odczytu2, 256);
				bzero(bufor_odczytu3, 256);	
			}
		}	
	}
	else
	{
		if(username==1)
		{
			printf("Nieudane logowanie na %s z IP:%s\n", login, inet_ntoa((struct in_addr)k->adres_k.sin_addr));
		}
		else
		{
			printf("Brak użytkownika o podanej nazwie z IP:%s\n", inet_ntoa((struct in_addr)k->adres_k.sin_addr));
			if(read(k->gniazdo_k, napis, 255)==-1) // Jeżeli użytkownik nie został znaleziony to nie ma żądania hasła od klienta
			{
			printf("Błąd odczytu z gniazda\n");
			close(k->gniazdo_k);
			free(k);
			return 0;
			}
		}
		if(write(k->gniazdo_k, "Nieudane logowanie\n", 19)==-1)
		{
			printf("Błąd wysyłania na gniazdo\n");
			close(k->gniazdo_k);
			free(k);
			return 0;
		}
	}
	if(write(k->gniazdo_k, "koniec", 6)==-1) 
	{	
		printf("Błąd zapisu na gniazdo dla IP:%s. Zamykam połączenie\n",inet_ntoa((struct in_addr)k->adres_k.sin_addr));
		close(k->gniazdo_k);
		free(k);	
		return 0;
	}
	printf("Zamknięto połączenie z %s\n", inet_ntoa((struct in_addr)k->adres_k.sin_addr));	
 	close(k->gniazdo_k);
	free(k);
	return 0;
}

int main(int argc, char* argv[])
{
	pthread_t tid;
  	int gniazdo_glowne = socket(PF_INET, SOCK_STREAM, 0);
  	int on = 1;
  	setsockopt(gniazdo_glowne, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
  	struct sockaddr_in adres;

  	adres.sin_family = PF_INET;
  	adres.sin_port= htons(PORT_NUMBER);
  	adres.sin_addr.s_addr = INADDR_ANY;
	
	int numer_portu = PORT_NUMBER;

  	if(bind(gniazdo_glowne, (struct sockaddr*)&adres, sizeof(adres))==-1)
	{
		printf("Nie udało się zbindować gniazda z adresami\n");
		exit(-1);
	}
  	listen(gniazdo_glowne, 10);
  	printf("Nasłuchuje na porcie %d\n", numer_portu);

  	while(1>0)
  	{
		struct klient* k = malloc(sizeof(struct klient));

  		socklen_t sl = sizeof(k->adres_k);
  		k->gniazdo_k = accept(gniazdo_glowne, (struct sockaddr*)&k->adres_k, &sl);
		pthread_create(&tid, NULL, watek_klienta, k);	
	 }
	close(gniazdo_glowne);
	return(0);
}
