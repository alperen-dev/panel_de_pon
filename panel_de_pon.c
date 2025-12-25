#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_GAME_MATRIX_SIZE 20
#define MIN_GAME_MATRIX_SIZE 3 /* 3x3'den az olduğunda herhangi bir patlama yapılamayacağı için 3 olarak belirledim, aslında dikey patlama için 4 yatay patlama için 3 olması gerekir yani (4x3) matris, şu anki haliyle sadece yatay patlama yapılabilir */

#define MAX_GAME_MATRIX_POSITON 20
#define MIN_GAME_MATRIX_POSITON 1

#define GAME_MODE_NORMAL 1
#define GAME_MODE_CONTROL 2

#define GAME_OPERATION_MOVE 1
#define GAME_OPERATION_EXPLODE 2

#define GAME_CHARACTERS_LENGTH 5 /* oyundaki toplam karakter çeşiti sayısı */

#define ABS(a) (((a) < 0) ? -(a) : (a)) /* koordinatların yan yana olup olmadığını kontrol etmek için koordinatların farklarının mutlak değerlerinin toplamının (yani ABS(x1-x2)+ABS(y1-2) değerinin ) 1'e eşit olup olmadığını kontrol etmek gerekiyor, bunu algoritma içinde uzun uzun if ile ifade etmek yerine makro kullanıldı */

/* terminali temizlemek için daha mantıklı yöntemler kullanılabilirdi ancak kodun taşınabilirliğini azaltmak durumunda kalınırdı */
void clear_screen() /* kimsenin terminali 1000 satırdan fazla değildir. . . umarım :) */
{
	int i = 0;
	for(i = 0; i < 1000; i++)
	{
		printf("\n");
	}
}


/* tablo yaparken | ve - karakterleri yerine direkt tablo karakterleri kullanılabilirdi ancak kodun taşınabilirliği azalırdı */
/* Çünkü modern sistemlerde kullanılan UNICODE'e göre tablo karakterleri ile eski tip (ANSI) tablo karakterlerinin karakter kodları farklı */
/* bu da UNICODE kullanan terminaller ile ANSI kullanan terminaller arasında taşınabilirliği azaltırdı */
void print_matrix(char matrix[][MAX_GAME_MATRIX_SIZE], int n, int m) /* genel bir fonksiyon değil, iyi bir görünüm için en fazla 20 sütun destekler */
{
	int i = 0, j = 0;
	
	/* sütunun onlar basamağı yazılır */
	printf("\n  |%-.*s|\n  ", m*2-1, "0                 1                   2"); /* sütun sayısında göre 3. parametredeki string'in belli bir kısmı yazılır */
	/* sütun numarasının onlar basamağı yazıldı */
	
	/* sütun numarasının birler basamağı yazılır */
	for(j = 0; j < m; j++)
	{
		printf("|%d", (j+1)%10);
	}
	printf("|\n");
	/* sütun numarasının birler basamağı yazıldı */
	
	for(j = 0; j < m*2+3; j++) /* satırların en üstüne tire işaretleri konulur */
	{
		printf("-");
	}
	printf("\n");
	
	
	for(i = 0; i < n; i++)
	{
		printf("%02d", i+1); /* her satırın başında satır numarası yaz */
		for(j = 0; j < m; j++)
		{
			printf("|%c", (matrix[i][j] == '\0') ? ' ' : matrix[i][j]); /* tablo karakterini ve oyun karakterini yaz */
		}
		printf("|\n"); /* tablo karakterinin en sonuncusunu koy */
		for(j = 0; j < m*2+3; j++) /* yazılan satırın altına tire işaterlerini koy */
		{
			printf("-");
		}
		printf("\n");
	}
}

int main()
{
	int n = 0, m = 0, temp = 0, is_valid = 0, game_mode = 0, operation = 0, movement_count = 0, explosion_count = 0, lost = 0, i = 0, j = 0, k = 0;
	char game_matrix[MAX_GAME_MATRIX_SIZE][MAX_GAME_MATRIX_SIZE], ch = 0;
	const char game_characters[GAME_CHARACTERS_LENGTH] = {'*', '/', '+', '%', 'O'};
	
	
	/* Kullanıcıdan matris boyutunun alınması ve kontrol edilmesi, geçersizse gerekli mesajların yazılması */
	is_valid = 0;
	do
	{
		printf("Oyun alanının boyutlarını (NxM) giriniz [%d-%d]: ", MIN_GAME_MATRIX_SIZE, MAX_GAME_MATRIX_SIZE);
		temp = scanf("%d %d", &n, &m);
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
		while((temp = getchar()) != '\n' && temp != EOF); /* stdin arabelleğini temizleme */
	}while(is_valid == 0);
	/* Kullanıcıdan matrisin boyunu alma işlemi bitti */
	
	
	/* Kullanıcıdan oyun modunun alınması ve kontrol edilmesi, gerekirse hata mesajlarının yazılması */
	is_valid = 0;
	do
	{
		printf("Oyun modu için %d, kontrol modu için %d giriniz: ", GAME_MODE_NORMAL, GAME_MODE_CONTROL);
		temp = scanf("%d", &game_mode);
		if(temp != 1)
		{
			printf("Geçersiz girdi!\n");
		}
		else if(game_mode != GAME_MODE_NORMAL && game_mode != GAME_MODE_CONTROL)
		{
			printf("Geçersiz mod!\n");
		}
		else
		{
			is_valid = 1;
		}
		while((temp = getchar()) != '\n' && temp != EOF); /* stdin arabelleğini temizleme */
	}while(is_valid == 0);
	/* Kullanıcıdan oyun modunu alma bitti */
	
	
	/* oyun modu ve kontol modu için game_matrix'e uygun değerlerin yerleştirilmesi */
	for(i = 0; i < n/2; i++) /* Eğer program birden fazla kez dönecek ise eski değerleri silmek için bu döngüye ihtiyaç var */
	{
		for(j = 0; j < m; j++)
		{
			game_matrix[i][j] = '\0';
		}
	}
	/* Modlara göre matrisi oluştur veya iste */
	if(game_mode == GAME_MODE_NORMAL)
	{
		srand(time(NULL)); /* rastgele sayı üretmek için zamanı kullandık. basit bir oyun için yeterli bir "rastgelelik" sağlar */
		for(i = n-n/2; i < n; i++) /* n-n/2 dedik eğer direkt n/2 deseydik "üstten n/2 boş bırak" demiş olurduk, bu da tek sayıda satır varsa bir satır daha fazla dolu olmasına sebep olurdu." */
		{
			for(j = 0; j < m; j++)
			{
				game_matrix[i][j] = game_characters[rand() % GAME_CHARACTERS_LENGTH]; /* rastgele bir indisdeki karakteri alır */
			}
		}
	}
	else /* game_mode == GAME_MODE_CONTROL, ancak kontrol etmeye gerek yok */
	{
		printf( "Kontrol modunu seçtiniz %d-%d satırlarının başlangıç durumlarını girmeniz gerekmektedir.\n"
				" Girebileceğiniz karakterler: [%.*s]\n", (n/2)+1, n, GAME_CHARACTERS_LENGTH, game_characters);
		for(i = n-n/2; i < n; i++) /* n-n/2 dedik eğer direkt n/2 deseydik "üstten n/2 boş bırak" demiş olurduk, bu da tek sayıda satır varsa bir satır daha fazla dolu olmasına sebep olurdu." */
		{
			for(j = 0; j < m; j++)
			{				
				/* Girdi kontrolü yapılıp eğer doğru karakter girildi ise atama yapılır, hatalı girdi ise hata mesajı verilir. */
				is_valid = 0;
				do
				{
					printf("%d. satır, %d. sütun karakterini giriniz: ", i+1, j+1);
					scanf("%c", &ch);
					k = 0;
					while((k < GAME_CHARACTERS_LENGTH) && (ch != game_characters[k]))
					{
						k++;
					}
					if(k < GAME_CHARACTERS_LENGTH) /* Geçerli girdi */
					{
						is_valid = 1;
					}
					else /* Geçersiz girdi */
					{
						printf("\'%c\' karakteri geçerli bir oyun karakteri değil!\n", ch);
					}				
					while((temp = getchar()) != '\n' && temp != EOF); /* stdin arabelleğini temizleme */
				}while(is_valid == 0);
				game_matrix[i][j] = ch; /* girdi kontrolü yapıldı, atama yapılabilir */
			}
		}
	}
	/* Modlara göre matrisi oluşturma işlemi bitti. */
	
	
	/* Oynun asıl döngüsü */
	do
	{
		printf("Toplam yer değişikliği: %d\nToplam patlatılan element: %d\n", movement_count, explosion_count);
		print_matrix(game_matrix, n, m);
		
		/* Kullanıcıdan yapılacak işlemin alınması ve kontrol edilmesi, gekerirse hata mesajlarının yazılması */
		is_valid = 0;
		do
		{
			printf( "Yer değişikliği için %d\n"
					"Patlama için %d giriniz: ", GAME_OPERATION_MOVE, GAME_OPERATION_EXPLODE);
			temp = scanf("%d", &operation);
			if(temp != 1)
			{
				printf("Geçersiz girdi!\n");
			}
			else if(operation != GAME_OPERATION_MOVE && operation != GAME_OPERATION_EXPLODE)
			{
				printf("Geçersiz işlem!\n");
			}
			else
			{
				is_valid = 1;
			}
			while((temp = getchar()) != '\n' && temp != EOF); /* stdin arabelleğini temizleme */
		}while(is_valid == 0);
		/* Kullanıcıdan yapılacak işlemi alma bitti */
		
		if(operation == GAME_OPERATION_MOVE) /* İki karakterin yerini değiştirmek istediğinde */
		{
			int row1 = 0, col1 = 0, row2 = 0, col2 = 0;
			/* Yerleri değişecek iki koordinat kullanıcıdan alınır */
			is_valid = 0;
			do
			{
				printf("Yerlerini değiştirmek istediğiniz iki koordinatı giriniz (r1,c1 r2,c2): ");
				temp = scanf("%d,%d%d,%d", &row1, &col1, &row2, &col2);
				if(temp != 4)
				{
					printf("Geçersiz girdi!\n");
				}
				else if(row1 < MIN_GAME_MATRIX_POSITON || row1 > MAX_GAME_MATRIX_POSITON)
				{
					printf("r1=%d değeri geçersiz aralıktadır!\n", row1);
				}
				else if(col1 < MIN_GAME_MATRIX_POSITON || col1 > MAX_GAME_MATRIX_POSITON)
				{
					printf("c1=%d değeri geçersiz aralıktadır!\n", col1);
				}
				else if(row2 < MIN_GAME_MATRIX_POSITON || row2 > MAX_GAME_MATRIX_POSITON)
				{
					printf("r2=%d değeri geçersiz aralıktadır!\n", row2);
				}
				else if(col2 < MIN_GAME_MATRIX_POSITON || col2 > MAX_GAME_MATRIX_POSITON)
				{
					printf("c2=%d değeri geçersiz aralıktadır!\n", col2);
				}
				else
				{
					is_valid = 1;
				}
				while((ch = getchar()) != '\n' && ch != EOF); /* stdin arabelleğini temizleme */
			}while(is_valid == 0);
			row1--, col1--, row2--, col2--;
			/* koordinatlar başarı ile alındı */
			
			
			/* Şimdi bu koordinatlar yer değiştirilebilir mi onu kontrol etmeliyiz */
			if(game_matrix[row1][col1] == '\0')
			{
				printf("Girdiğiniz 1. koordinatlarda (%d,%d) herhangi bir karakter bulunmamaktadır!\n", row1+1, col1+1);
			}
			else if(game_matrix[row2][col2] == '\0')
			{
				printf("Girdiğiniz 2. koordinatlarda (%d,%d) herhangi bir karakter bulunmamaktadır!\n", row2+1, col2+1);
			}
			else if(ABS(row1-row2)+ABS(col1-col2) != 1) /* koordinatlar yan yana değil, sadece yan yana koordinatlar yer değiştirebilir */
			{
				printf("(%d,%d) koordinatları ile (%d,%d) koordinatları yan yana değil!\n", row1+1, col1+1, row2+1, col2+1);
			}
			else /* Geçerli koordinatlar, yer değiştirme işlemini yap */
			{
				ch = game_matrix[row1][col1];
				game_matrix[row1][col1] = game_matrix[row2][col2];
				game_matrix[row2][col2] = ch;
				
				movement_count++;
				for(j = 0; j < m; j++) /* matrisin en altına yeni satırı ekleme döngüsü, her sütuna tek tek eklenir */
				{
					/* matrisin en alt satırından üst satırlara doğru kaydırma işlemi yapılır. */
					/* en üstten başlamak daha az değişken kullanmamızı sağlardı ancak bu sefer önce boş olmayan satırı bulmamız gerekirdi (veya boş yere işlem gücü harcardık boş satırları kaydırarak) */
					temp = game_matrix[n-1][j];
					i = n-2;
					while((i >= 0) && (temp != '\0')) /* TODO: i >= 0 kontrolüne gerek olup olmadığını kontrol et */
					{
						int old_val = game_matrix[i][j];
						game_matrix[i][j] = temp;
						temp = old_val;
						i--;
					}
				}
				/* kaydırma işlemi bitti */
				/* sadece en üst satırı kontrol ederek oynun bitip bitmediğine karar verebiliriz */
				/* proje ön bilgisinde "Oyun tüm alan doluncaya" kadar denmiş ancak örnekte üst satırda birkaç tane sütun dolduğunda */
				/* oyun sonlandırılmış. Ben de bana daha mantıklı gelen üst satırda herhangi dolu bir kutu varsa oyun bitsin */
				/* şeklinde yapacağım */
				j = 0;
				while((j < m) && (game_matrix[0][j] == '\0')) /* üst satırda boş karakter yoksa döngüyü erken bitirir */
				{
					j++;
				}
				if(j < m) /* en üst satırda 0 olmayan eleman var */
				{
					lost = 1; /* yani kaybettin */
				}
			}
		}
		/* patlatmak */
		else /* operation == GAME_OPERATION_EXPLODE, ancak başka bir şey olamayacağı için direkt else dedik */
		{
			int row, col;
			
			/* Patlatılacak koordinat kullanıcıdan alınır */
			is_valid = 0;
			do
			{
				printf("Patlatmak istediğiniz bloğun sol veya üst koordinatını giriniz (r,c): ");
				temp = scanf("%d,%d", &row, &col);
				if(temp != 2)
				{
					printf("Geçersiz girdi!\n");
				}
				else if(row < MIN_GAME_MATRIX_POSITON || row > MAX_GAME_MATRIX_POSITON)
				{
					printf("r=%d değeri geçersiz aralıktadır!\n", row);
				}
				else if(col < MIN_GAME_MATRIX_POSITON || col > MAX_GAME_MATRIX_POSITON)
				{
					printf("c=%d değeri geçersiz aralıktadır!\n", col);
				}
				else
				{
					is_valid = 1;
				}
				while((temp = getchar()) != '\n' && temp != EOF); /* stdin arabelleğini temizleme */
			}while(is_valid == 0);
			row--, col--;
			/* koordinat başarı ile alındı */
			
			/* Şimdi bu koordinatta patlama olabilir mi onu kontrol etmeliyiz */
			/* proje kapsamında patlatmak için elemanın sol üst köşe olduğu belirtildiği için */
			/* sadece sağa ve aşağı doğru kontrol etmek yeterlidir. */
			if(game_matrix[row][col] == '\0')
			{
				printf("Girdiğiniz koordinatta (%d,%d) herhangi bir karakter bulunmamaktadır!\n", row+1, col+1);
			}
			else
			{
				int right_length = 1, down_length = 1; /* hangisi daha fazlaysa o kullanılacak, 1'den başlattık çünkü kendisi de arıyor */
				ch = game_matrix[row][col]; /* aradığımız elemanı kısaltmak için */
				
				/* mesafelerin tespit edilmesi */
				for(j = col+1; j < m && ch == game_matrix[row][j]; j++) /* sağa doğru */
				{
					right_length++;
				}
				for(i = row+1; i < n && ch == game_matrix[i][col]; i++) /* aşağı doğru */
				{
					down_length++;
				}
				
				/* Herhangi bir patlama olamadığı durum için */
				if((down_length < 3) && (right_length < 3))
				{
					printf("Herhangi bir patlama olamadı!\nDevam etmek için bir ENTER (Return) tuşuna basın ");
					getchar();
					while((temp = getchar()) != '\n' && temp != EOF); /* stdin arabelleğini temizleme */
				}
				
				
				/* mesafesi fazla olan ve 3'den büyük eşit olan yöne doğru patlatma işlemini yap */
				else if(right_length >= down_length) /* > değil >= çünkü satır patlarmak oyunun daha uzun süre bitmemesini sağlayabilir (bence) */
				{
					for(j = 0; j < right_length; j++) /* daha önceden bulunan uzuluk kadar sütunun üstünü 1 aşağı aldık */
					{
						for(i = row-1; i >= 0 && game_matrix[i][col+j] != '\0'; i--)
						{
							game_matrix[i+1][col+j] = game_matrix[i][col+j]; /* satırı birer birer aşağı kaydır */
						}
						game_matrix[i+1][col+j] = '\0'; /* aşağı düşmeden önce en üstte bulunan satır artık bir aşağıda, bunu 0 yapmalıyız. */
						explosion_count += right_length; /* patlama gerçekleşti */
					}
				}
				else /* Burada bir şeyi kontrol etmemize gerek yok zaten ilk iki if'in olmama durumunda aşağı uzunluk sağ uzunluktan büyük ve aşağı uzunluk yeterli bir uzunluğu sahip */
				{
					for(i = row-1; i >= 0 && game_matrix[i][col] != '\0'; i--) /* daha önceden bulunan uzunluk (down_length) kadar aşağı doğru satıları kaydıracağız */
					{
						game_matrix[i+down_length][col] = game_matrix[i][col]; /* üstteki satırı down_length kadar aşağı al */
						game_matrix[i][col] = '\0'; /* bu satır artık aşağıda, buradan silebiliriz */
					}
					explosion_count += down_length; /* patlama gerçekleşti */
				}
				/* kaydırma işlemi bitti */
				/* ödevin herhangi bir yerinde "kazanmak" diye bir şey geçmediği için bu kadar */
				/* eğer tüm elemanlar bitince kazanmak olsaydı onu da son satırı kontrol ederek yapabilirdik. */
				
			}
		}
	}while(lost == 0);
	
	return 0;
}