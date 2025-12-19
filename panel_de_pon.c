#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_GAME_MATRIX_SIZE 20
#define MIN_GAME_MATRIX_SIZE 1

int main()
{
	int n = 0, m = 0, temp = 0, is_valid = 0, game_mode = 0, game_matrix[MAX_GAME_MATRIX_SIZE][MAX_GAME_MATRIX_SIZE];
	
	
	/* Kullanıcıdan matris boyutunun alınması ve kontrol edilmesi, geçersizse gerekli mesajların yazılması */
	is_valid = 0;
	do
	{
		printf("Oyun alanının boyutlarını (NxM) giriniz [%d-%d]: ", 1, MAX_GAME_MATRIX_SIZE);
		temp = scanf("%d%d%*[^\n]", &n, &m);
		if(temp != 2)
		{
			printf("Geçersiz girdi!\n");
		}
		else if(n < MIN_GAME_MATRIX_SIZE || n > MAX_GAME_MATRIX_SIZE)
		{
			printf("N=%d değeri geçersiz aralıktadır!\n", n);
		}
		else if(m < MIN_GAME_MATRIX_SIZE || m > MAX_GAME_MATRIX_SIZE)
		{
			printf("M=%d değeri geçersiz aralıktadır!\n", m);
		}
		else
		{
			is_valid = 1;
		}
		//while((getchar()) != '\n'); /* stdin arabelleğini temizleme */
	}while(is_valid == 0);
	/* Kullanıcıdan matrisin boyunu alma işlemi bitti */
	
	is_valid = 0;
	do
	{
		printf("Oyun alanının boyutlarını (NxM) giriniz [%d-%d]: ", 1, MAX_GAME_MATRIX_SIZE);
		temp = scanf("%d %d%*c", &n, &m);
		if(temp != 1)
		{
			printf("Geçersiz girdi!\n");
			while(getchar() != '\n');
		}
		else if(n < MIN_GAME_MATRIX_SIZE || n > MAX_GAME_MATRIX_SIZE)
		{
			printf("N=%d değeri geçersiz aralıktadır!\n", n);
		}
		else if(m < MIN_GAME_MATRIX_SIZE || m > MAX_GAME_MATRIX_SIZE)
		{
			printf("M=%d değeri geçersiz aralıktadır!\n", m);
		}
		else
		{
			is_valid = 1;
		}
	}while(is_valid == 0);
	/* Kullanıcıdan matrisin boyunu alma işlemi bitti */
	
	return 0;
}