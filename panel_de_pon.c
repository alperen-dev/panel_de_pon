#include <stdio.h> /* printf scanf getchar */
#include <stdlib.h> /* rastgele sayı üretmek için */
#include <time.h> /* rastgele sayı üretirken seed'i zamana göre seçebilmek için */

#define MAX_GAME_MATRIX_SIZE 20
#define MIN_GAME_MATRIX_SIZE 3 /* 3x3'den az olduğunda herhangi bir patlama yapılamayacağı için 3 olarak belirledim, aslında dikey patlama için 4 yatay patlama için 3 olması gerekir yani (4x3) matris, şu anki haliyle sadece yatay patlama yapılabilir */

#define MAX_GAME_MATRIX_POSITON 20
#define MIN_GAME_MATRIX_POSITON 1

#define GAME_MODE_NORMAL 1
#define GAME_MODE_CONTROL 2

#define GAME_OPERATION_EXIT -1
#define GAME_OPERATION_MOVE 1
#define GAME_OPERATION_EXPLODE 2

#define GAME_CHARACTERS_LENGTH 5 /* oyundaki toplam karakter çeşiti sayısı */

#define ABS(a) (((a) < 0) ? -(a) : (a)) /* koordinatların yan yana olup olmadığını kontrol etmek için koordinatların farklarının mutlak değerlerinin toplamının (yani ABS(x1-x2)+ABS(y1-2) değerinin ) 1'e eşit olup olmadığını kontrol etmek gerekiyor, bunu algoritma içinde uzun uzun if ile ifade etmek yerine makro kullanıldı */

/* Oyun ilk açıldığında ekrana yazılacak yazıları barındıran fonksiyon */
void print_splash_screen()
{
	puts(
			"Panel De Pon oynuna hoş geldiniz!\n"
			"Bu oynun amacı karakterler en üst satıra değmeden en fazla sayıda element patlatmaya çalışmaktır\n"
			"Oyun karakterleri: [*/+%O]\n"
			"\n"
			"Oyun kuralları: \n"
			" 1- İki elementi yer değiştirmek istediğinizde sadece komşu iki elementlerin yerlerini değiştirebilirsiniz\n"
			" 2- İki elementin yerleri değiştirildiğinde tüm satırlar birer yukarı kayar ve en alt satıra rastgele elementler eklenir\n"
			" 3- En üst satıra herhangi bir element ulaştığında oyun direkt sona erer\n"
			" 4- Bir element grubunu patlatabilmek için en az 3 tane yan yana veya alt alta aynı elementten bulunmalıdır\n"
			" 5- Eğer hem dikey hem yatay patlama olabiliyorsa oyun en fazla element patlayabilecek olanı seçecektir\n"
			" 6- Eğer dikey ve yatay patlayabilecek element sayısı eşitse oyun yatay olanı seçer\n"
			" 7- Bir element grubunu patlatmaya karar verdiğinizde bu grubun en üst veya en sol elementinin koordinatını girmeniz gerekmektedir\n"
			"\n"
			"Oyun Modları: \n"
			" 1- Normal Mod:\n"
			"  Bu modda oyun matrisinin alttan n/2 kadar satırı rastgele elementlerle doldurulur.\n"
			"  Her hamleden sonra oyun ekranı temizlenir\n"
			" 2- Kontrol Mod:\n"
			"  Bu modda oyun matrisinin alttan n/2 kadar satırı ve M kadar sütununda bulunan elementler kullanıcıdan istenir\n"
		);
}

/* fgets dışında bir yöntemle input aldığın zaman bunu kullanmak gerekir */
/* fgets \n dahil olmak üzere tüm stdin bufferini kendine alır */
/* ancak scanf, getchar gibi fonksiyonlar input buffer'deki tüm değerleri almaz */
/* bu yüzden bu fonksiyonları çağırdığımızda stdin bufferinde \n gibi çöp değerler kalabilir */
/* bu fonksiyonun amacı ise stdin'deki bu çöp değerleri temizlemektir */
void clear_stdin()
{
	int temp;
	while((temp = getchar()) != '\n' && temp != EOF);
}

/* terminali temizlemek için daha mantıklı yöntemler kullanılabilirdi ancak kodun taşınabilirliğini azaltmak durumunda kalınırdı */
void clear_screen() /* kimsenin terminali 1000 satırdan fazla değildir. . . umarım :) */
{
	int i = 0;
	for(i = 0; i < 1000; i++)
	{
		printf("\n");
	}
}


/* tablo yaparken |-+ karakterleri yerine direkt tablo karakterleri kullanılabilirdi ancak kodun taşınabilirliği azalırdı */
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


/* mantık olarak powershell'deki pause komutuna benzer bir fonksiyon, enter tuşuna basana kadar bekler */
void pause() 
{
	printf("Devam etmek için Enter (Return) tuşuna basın . . .\n");
	clear_stdin(); /* bu fonksyion zaten getchar'ı çağırdığı için bir daha getchar çağırmamıza gerek yok */
}

/* Oyunun o anki durumunu yazan fonksiyon, bu fonksiyonun kullanılma sebebi oyun döngüsünde birden fazla yerde */
/* oyun matrisini ve puanları yazdırmamızın gerekmesi. parametre olarak verilen değerlere ve moda göre ekranda */
/* yazılacak metinleri ayarlar. */
void print_status(char matrix[][MAX_GAME_MATRIX_SIZE], int n, int m, int game_mode, int movement_count, int explosion_count)
{
	if(game_mode == GAME_MODE_NORMAL) /* eğer oyun modu ise ekranı temizle */
	{
		clear_screen();
	}
	/* durumu ekrana yaz */
	printf("\nToplam yer değişikliği: %d\nToplam patlatılan element: %d\n", movement_count, explosion_count);
	print_matrix(matrix, n, m);
}

int main()
{
	int n = 0, m = 0, temp = 0, is_valid = 0, game_mode = 0, operation = 0, movement_count = 0, explosion_count = 0, ended = 0, i = 0, j = 0, k = 0;
	char game_matrix[MAX_GAME_MATRIX_SIZE][MAX_GAME_MATRIX_SIZE], ch = 0;
	const char game_characters[GAME_CHARACTERS_LENGTH] = {'*', '/', '+', '%', 'O'};
	
	srand(time(NULL)); /* rastgele sayı üretmek için zamanı kullandık. basit bir oyun için yeterli bir "rastgelelik" sağlar */
	
	print_splash_screen();
	
	/* Kullanıcıdan matris boyutunun alınması ve kontrol edilmesi, geçersizse gerekli mesajların yazılması */
	do
	{
		is_valid = 1; /* Girdinin en başta doğru olduğu varsayılır */
		printf("Oyun alanının boyutlarını (N M) giriniz [%d-%d]: ", MIN_GAME_MATRIX_SIZE, MAX_GAME_MATRIX_SIZE);
		temp = scanf("%d %d", &n, &m);
		clear_stdin();
		if(temp != 2)
		{
			printf("Geçersiz girdi, bu girdi için 2 tane [%d-%d] arası tamsayı girmelisiniz!\n", MIN_GAME_MATRIX_SIZE, MAX_GAME_MATRIX_SIZE);
			is_valid = 0;
		}
		else /* girdi sayısı doğru bu yüzden n ve m'nin değerlerini kontrol edebiliriz */
		{
			if(n < MIN_GAME_MATRIX_SIZE || n > MAX_GAME_MATRIX_SIZE)
			{
				printf("N=%d değeri geçersiz aralıktadır!\n", n);
				is_valid = 0;
			}
			if(m < MIN_GAME_MATRIX_SIZE || m > MAX_GAME_MATRIX_SIZE)
			{
				printf("M=%d değeri geçersiz aralıktadır!\n", m);
				is_valid = 0;
			}
		}
		
		if(is_valid == 0) /* girdi hatalıysa kullanıcıyı bekle */
		{
			pause();
		}
		
	}while(is_valid == 0);
	/* Kullanıcıdan matrisin boyunu alma işlemi bitti */
	
	
	/* Kullanıcıdan oyun modunun alınması ve kontrol edilmesi, gerekirse hata mesajlarının yazılması */
	do
	{
		is_valid = 1;
		printf("Oyun modu için %d, kontrol modu için %d giriniz: ", GAME_MODE_NORMAL, GAME_MODE_CONTROL);
		temp = scanf("%d", &game_mode);
		clear_stdin();
		if(temp != 1)
		{
			printf("Geçersiz girdi, bu girdi için %d veya %d tamsayı değerlerini girmelisiniz!\n", GAME_MODE_NORMAL, GAME_MODE_CONTROL);
			is_valid = 0;
			pause();
		}
		else if(game_mode != GAME_MODE_NORMAL && game_mode != GAME_MODE_CONTROL)
		{
			printf("Geçersiz mod, geçerli modlar: %d ve %d\n", GAME_MODE_NORMAL, GAME_MODE_CONTROL);
			is_valid = 0;
			pause();
		}
	}while(is_valid == 0);
	/* Kullanıcıdan oyun modunu alma bitti */
	
	
	/* game_matrix'in boş kalması gereken kısımlarına '\0' değeri atanır */
	for(i = 0; i < n-n/2; i++) /* Eğer program birden fazla kez dönecek ise eski değerleri silmek için de bu döngüye ihtiyaç var */
	{
		for(j = 0; j < m; j++)
		{
			game_matrix[i][j] = '\0';
		}
	}
	
	/* Modlara göre matrisi oluştur veya iste */
	if(game_mode == GAME_MODE_NORMAL)
	{
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
				"Girebileceğiniz karakterler: [%.*s]\n", n-(n/2)+1, n, GAME_CHARACTERS_LENGTH, game_characters);
		for(i = n-n/2; i < n; i++) /* n-n/2 dedik eğer direkt n/2 deseydik "üstten n/2 boş bırak" demiş olurduk, bu da tek sayıda satır varsa bir satır daha fazla dolu olmasına sebep olurdu." */
		{
			for(j = 0; j < m; j++)
			{				
				/* Girdi kontrolü yapılıp eğer doğru karakter girildi ise atama yapılır, hatalı girdi ise hata mesajı verilir. */
				do
				{
					is_valid = 1;
					printf("%d. satır, %d. sütun karakterini giriniz: ", i+1, j+1);
					scanf(" %c", &ch); /* %c değerinde bulunan boşluk kullanıcının hatalı girdi yapmasını önleyebilir. (girilen karakterden önceki white spaceleri atlayarak) */
					clear_stdin();
					k = 0;
					while((k < GAME_CHARACTERS_LENGTH) && (ch != game_characters[k])) /* girilen karakterin oyun karakteri olup olmadığına karar veren döngü */
					{
						k++;
					}
					if(k >= GAME_CHARACTERS_LENGTH) /* Geçersiz girdi */
					{
						printf( "\'%c\' karakteri geçerli bir oyun karakteri değil!\n"
								"Geçerli oyun karakterleri şunlardır: [%.*s]\n", ch, GAME_CHARACTERS_LENGTH, game_characters);
						is_valid = 0;
						pause();
					}				
				}while(is_valid == 0);
				game_matrix[i][j] = ch; /* girdi kontrolü yapıldı, atama yapılabilir */
			}
		}
	}
	/* Modlara göre matrisi oluşturma işlemi bitti. */
	
	
	/* Oynun asıl döngüsü */
	do
	{
		print_status(game_matrix, n, m, game_mode, movement_count, explosion_count);
		
		/* Kullanıcıdan yapılacak işlemin alınması ve kontrol edilmesi, gekerirse hata mesajlarının yazılması */
		do
		{
			is_valid = 1;
			printf( "Yer değişikliği için %d\n"
					"Patlama için %d\n"
					"Oyundan çıkmak için %d giriniz: ", GAME_OPERATION_MOVE, GAME_OPERATION_EXPLODE, GAME_OPERATION_EXIT);
			temp = scanf("%d", &operation);
			clear_stdin();
			if(temp != 1)
			{
				printf("Geçersiz girdi, bu girdi için %d, %d veya %d tamsayı değerlerini girmelisiniz!\n", GAME_OPERATION_MOVE, GAME_OPERATION_EXPLODE, GAME_OPERATION_EXIT);
				is_valid = 0;
			}
			else if(operation != GAME_OPERATION_MOVE && operation != GAME_OPERATION_EXPLODE && operation != GAME_OPERATION_EXIT)
			{
				printf("Geçersiz işlem, geçerli işlemler: {%d, %d, %d}\n", GAME_OPERATION_MOVE, GAME_OPERATION_EXPLODE, GAME_OPERATION_EXIT);
				is_valid = 0;
			}
			
			if(is_valid == 0)
			{
				pause();
			}
		}while(is_valid == 0);
		/* Kullanıcıdan yapılacak işlemi alma bitti */
		
		if(operation == GAME_OPERATION_EXIT) /* Oyundan çıkılmak istendiğinde */
		{
			printf("Oyun kullanıcı isteği ile sonlandırılmıştır.\n");
			ended = 1;
		}
		else if(operation == GAME_OPERATION_MOVE) /* İki karakterin yerini değiştirmek istediğinde */
		{
			int row1 = 0, col1 = 0, row2 = 0, col2 = 0;
			int operation_cancelled = 0;
			/* Yerleri değişecek iki koordinat kullanıcıdan alınır */
			do
			{
				is_valid = 1; /* girdinin en başta doğru olduğu varsayıldı */
				printf("Yerlerini değiştirmek istediğiniz iki koordinatı giriniz (format: r1,c1 r2,c2): ");
				temp = scanf("%d , %d %d , %d", &row1, &col1, &row2, &col2);
				clear_stdin();
				if(temp != 4)
				{
					operation_cancelled = 1;
					printf("Yer değiştirme işlemi iptal edildi\n");
					pause();
				}
				else
				{
					/* burada hataları else if.. diye değil de direkt if ile kontrol etmemizin sebebi */
					/* eğer birden fazla yanlış koordinat varsa hepsini birden kullanıcıya haber etsin için */
					if(row1 < MIN_GAME_MATRIX_POSITON || row1 > MAX_GAME_MATRIX_POSITON)
					{
						is_valid = 0;
						printf("r1=%d değeri geçersiz aralıktadır!\n", row1);
					}
					if(col1 < MIN_GAME_MATRIX_POSITON || col1 > MAX_GAME_MATRIX_POSITON)
					{
						is_valid = 0;
						printf("c1=%d değeri geçersiz aralıktadır!\n", col1);
					}
					if(row2 < MIN_GAME_MATRIX_POSITON || row2 > MAX_GAME_MATRIX_POSITON)
					{
						is_valid = 0;
						printf("r2=%d değeri geçersiz aralıktadır!\n", row2);
					}
					if(col2 < MIN_GAME_MATRIX_POSITON || col2 > MAX_GAME_MATRIX_POSITON)
					{
						is_valid = 0;
						printf("c2=%d değeri geçersiz aralıktadır!\n", col2);
					}
					if(is_valid == 1) /* koordinatlar geçerli aralıkta, şimdi koordinatların yer değiştirip değiştiremeyeceğini kontrol et */
					{
						if(game_matrix[row1-1][col1-1] == '\0')
						{
							is_valid = 0;
							printf("Girdiğiniz 1. koordinatlarda (%d,%d) herhangi bir karakter bulunmamaktadır!\n", row1, col1);
						}
						if(game_matrix[row2-1][col2-1] == '\0')
						{
							is_valid = 0;
							printf("Girdiğiniz 2. koordinatlarda (%d,%d) herhangi bir karakter bulunmamaktadır!\n", row2, col2);
						}
						if(row1 != row2 || ABS(col1-col2) != 1) // koordinatlar yan yana, sadece yan yana koordinatlar yer değiştirebilir
						{
							is_valid = 0;
							printf("(%d,%d) koordinatları ile (%d,%d) koordinatları yan yana değil!\n", row1, col1, row2, col2);
						}
						/*else if(ABS(row1-row2)+ABS(col1-col2) != 1) // koordinatlar yan yana veya alt alta değil, sadece yan yana veya alt alta koordinatlar yer değiştirebilir
						{
							is_valid = 0;
							printf("(%d,%d) koordinatları ile (%d,%d) koordinatları yan yana veya alt alta değil!\n", row1, col1, row2, col2);
						}*/
					}
				}
				if(is_valid == 0) /* hata oldu, kullanıcıya bilgi verildi, bekle */
				{
					pause();
					print_status(game_matrix, n, m, game_mode, movement_count, explosion_count);
				}
			}while(is_valid == 0);
			/* koordinatlar başarı ile alındı */
					
			if(operation_cancelled == 0) /* Geçerli koordinatlar, yer değiştirme işlemini yap */
			{
				row1--, col1--, row2--, col2--;
				
				ch = game_matrix[row1][col1];
				game_matrix[row1][col1] = game_matrix[row2][col2];
				game_matrix[row2][col2] = ch;
				
				movement_count++;
				
				/* Burada yer değiştirme işlemi yapıldıktan sonra, ama aşağı yeni satır eklemeden önce bizden matrisi yazdırmamız istendiği için */
				print_status(game_matrix, n, m, game_mode, movement_count, explosion_count);
				printf("Yer değiştirildi. Yeni satır ekleniyor\n");
				pause();
				
				
				
				for(j = 0; j < m; j++) /* matrisin en altına yeni satırı ekleme döngüsü, her sütuna tek tek eklenir */
				{
					/* matrisin en alt satırından üst satırlara doğru kaydırma işlemi yapılır. */
					/* en üstten başlamak daha az değişken kullanmamızı sağlardı ancak bu sefer önce boş olmayan satırı bulmamız gerekirdi (veya boş yere işlem gücü harcardık boş satırları kaydırarak) */
					temp = game_matrix[n-1][j];
					i = n-2;
					while(temp != '\0') /* TODO: i >= 0 kontrolüne gerek olup olmadığını kontrol et */
					{
						int old_val = game_matrix[i][j];
						game_matrix[i][j] = temp;
						temp = old_val;
						i--;
					}
					/* sütundaki her satır bir üste kaydırıldı, şimdi rastgele yeni bir karakteri en alta yaz */
					game_matrix[n-1][j] = game_characters[rand() % GAME_CHARACTERS_LENGTH];
					/* en üst satırı kontrol et, eğer bir taş varsa ended = 1 */
					if(game_matrix[0][j] != '\0')
					{
						ended = 1; /* yani kaybettin */
					}
				}
				/* kaydırma işlemi bitti */
				/* proje ön bilgisinde "Oyun tüm alan doluncaya" kadar denmiş ancak örnekte üst satırda birkaç tane sütun dolduğunda */
				/* oyun sonlandırılmış. Ben de bana daha mantıklı gelen üst satırda herhangi dolu bir kutu varsa oyun bitsin şeklinde yaptım*/
				if(ended == 1) /* en üst satırda 0 olmayan eleman var */
				{
					print_status(game_matrix, n, m, game_mode, movement_count, explosion_count);
					printf("Oyun en üst satırda eleman bulunduğu için sonlandırılmıştır.");
					
				}
			}
		}
		/* patlatmak */
		else /* operation == GAME_OPERATION_EXPLODE, ancak başka bir şey olamayacağı için direkt else dedik */
		{
			int row, col;
			int operation_cancelled = 0;
			
			/* Patlatılacak koordinat kullanıcıdan alınır */
			do
			{
				is_valid = 1; /* girdinin en başta doğru olduğu varsayıldı */
				printf("Patlatmak istediğiniz bloğun sol veya üst koordinatını giriniz (format: r,c): ");
				temp = scanf("%d , %d", &row, &col);
				clear_stdin();
				if(temp != 2)
				{
					operation_cancelled = 1;
					printf("Patlatma işlemi iptal edildi\n");
					pause();
				}
				else
				{
					if(row < MIN_GAME_MATRIX_POSITON || row > MAX_GAME_MATRIX_POSITON)
					{
						is_valid = 0;
						printf("r=%d değeri geçersiz aralıktadır!\n", row);
					}
					if(col < MIN_GAME_MATRIX_POSITON || col > MAX_GAME_MATRIX_POSITON)
					{
						is_valid = 0;
						printf("c=%d değeri geçersiz aralıktadır!\n", col);
					}
					/* önce is_valid kontrol ettik ki eğer sınır dışında ise rastgele bellek adresine erişmeye çalışmasın */
					if(is_valid == 1 && game_matrix[row-1][col-1] == '\0') /* boş kutu patlatılamaz */
					{
						is_valid = 0;
						printf("Girdiğiniz koordinatta (%d,%d) herhangi bir karakter bulunmamaktadır!\n", row, col);
					}
				}
				if(is_valid == 0) /* hata oldu, kullanıcıya bilgi ver */
				{
					pause();
					print_status(game_matrix, n, m, game_mode, movement_count, explosion_count);
				}
			}while(is_valid == 0);
			
			/* eğer operasyon iptal edilmediyse (karakter girmek gibi) işleme devam edebiliriz*/
			/* operation_cancelled == 1 ise patlatma işlemi iptal edilmiş demektir */
			if(operation_cancelled == 0) /* iptal edilmedi */
			{
				/* girdi başarıyla alındı */
				row--, col--;
				/* Şimdi bu koordinatta patlama olabilir mi onu kontrol etmeliyiz */
				/* proje kapsamında patlatmak için elemanın sol üst köşe olduğu belirtildiği için */
				/* sadece sağa ve aşağı doğru kontrol etmek yeterlidir. */
				int right_length = 1, down_length = 1; /* hangisi daha fazlaysa o kullanılacak, 1'den başlattık çünkü kendisi de arıyor */
				ch = game_matrix[row][col]; /* aradığımız elemanı kısaltmak için (önbellek optimizasyonu) */
				
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
					printf("Herhangi bir patlama olamadı!\n");
					pause();
				}
				/* mesafesi fazla olan ve 3'den büyük eşit olan yöne doğru patlatma işlemini yap */
				else if(right_length >= down_length) /* > değil >= çünkü satır patlarmak oyunun daha uzun süre bitmemesini sağlayabilir (bence) */
				{
					for(j = 0; j < right_length; j++) /* daha önceden bulunan uzuluk kadar sütunun üstünü 1 aşağı aldık */
					{
						for(i = row-1; i >= 0 && game_matrix[i][col+j] != '\0'; i--) /* sütunun dolu olduğu elemanları birer birer aşağı ötele */
						{
							game_matrix[i+1][col+j] = game_matrix[i][col+j]; /* satırı birer birer aşağı kaydır */
						}
						game_matrix[i+1][col+j] = '\0'; /* aşağı düşmeden önce en üstte bulunan satır artık bir aşağıda, bunu 0 yapmalıyız. */
					}
					explosion_count += right_length; /* patlama gerçekleşti */
				}
				else /* Burada bir şeyi kontrol etmemize gerek yok zaten ilk iki if'in olmama durumunda aşağı uzunluk sağ uzunluktan büyük ve aşağı uzunluk yeterli bir uzunluğu sahip */
				{
					/* bu dögüde k indisi patlama olan bölgenin üstündeki elemanları, i indisi patlama olan bölgeleri temsil eder */
					k = row-1;
					for(i = row-1+down_length; game_matrix[k][col] != '\0'; i--, k--)
					{
						game_matrix[i][col] = game_matrix[k][col]; /* üstteki satırı down_length kadar aşağı al */
					}
					while(i < k) /* aşağı düşen satırları '\0' değeri atadık */
					{
						game_matrix[i][col] = '\0';
						i--;
					}
					explosion_count += down_length; /* patlama gerçekleşti */
				}
				/* kaydırma işlemi bitti */
				/* ödevin herhangi bir yerinde "kazanmak" diye bir şey geçmediği için bu kadar */
				/* eğer tüm elemanlar bitince kazanmak olsaydı onu da son satırı kontrol ederek yapabilirdik. */
			}
		}
	}while(ended == 0);
	
	return 0;
}