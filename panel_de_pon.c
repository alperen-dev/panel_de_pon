#include <stdio.h> /* printf scanf getchar */
#include <stdlib.h> /* rastgele sayı üretmek için */
#include <time.h> /* rastgele sayı üretirken seed'i zamana göre seçebilmek için */

#define MAX_GAME_MATRIX_SIZE 20
#define MIN_GAME_MATRIX_SIZE 3 /* 3x3'den az olduğunda herhangi bir patlama yapılamayacağı için 3 olarak belirledim, aslında dikey patlama için 4 yatay patlama için 3 olması gerekir yani (4x3) matris, şu anki haliyle sadece yatay patlama yapılabilir */

#define MAX_GAME_MATRIX_POSITON 20
#define MIN_GAME_MATRIX_POSITON 1

#define GAME_CHARACTERS_LENGTH 5 /* oyundaki toplam karakter çeşiti sayısı */

/* Oyun ön bilgisindeki n/2 satırının indisini bulan makro */
#define MIDDLE_ROW(n) ((n)-(n)/2)

/* tek global değişken bu, tüm oyunda standart olarak kullanılacağı için global olarak tanımlandı */
const char game_characters[GAME_CHARACTERS_LENGTH] = {'*', '/', '+', '%', 'O'};

/* oyun matrisini tanımlarken kullanacağımız tipi tanımladık (direkt char game_matrix[][20] diyebilirdik, daha temiz durması için) */
typedef char GameMatrix[MAX_GAME_MATRIX_SIZE][MAX_GAME_MATRIX_SIZE];

/* Oyun modlarının alabileceği değerler */
typedef enum GameMode
{
	GAME_MODE_UNKNOWN = 0, /* henüz bir değer almadığını göstermek için */
	GAME_MODE_NORMAL = 1,
	GAME_MODE_CONTROL = 2
}GameMode;

/* Oyun döngüsünde kullanıcının yapabileceği işlemler */
typedef enum GameOperation
{
	GAME_OPERATION_EXIT = -1,
	GAME_OPERATION_UNKNOWN = 0, /* henüz bir değer almadığını göstermek için */
	GAME_OPERATION_MOVE = 1,
	GAME_OPERATION_EXPLODE = 2
}GameOperation;

typedef struct MatrixSize
{
	int n, m; /* n->row, m->column */
}MatrixSize;

typedef struct Coord
{
	int i, j; /* i->row, j->column */
}Coord;

typedef struct MovementCoords
{
	Coord c1;
	Coord c2;
}MovementCoords;

typedef struct GameStatus
{
	int gameMode, movementCount, explosionCount;
}GameStatus;

/* Oyun ilk açıldığında ekrana yazılacak yazıları barındıran fonksiyon */
void print_splash_screen()
{
	/* stringleri parçalara ayırdık çünkü bu kadar uzun bir string c89 (anci c) tarafından desteklenmez (null hariç en fazla 509 karakter) */
	puts(
			"Panel De Pon oyununa hoş geldiniz!\n"
			"Bu oynun amacı karakterler en üst satıra değmeden en fazla sayıda element patlatmaya çalışmaktır\n"
			"Oyun karakterleri: [*/+%O]\n"
			"\n"
			"Oyun kuralları: \n"
			" 1- İki elementi yer değiştirmek istediğinizde sadece komşu iki elementlerin yerlerini değiştirebilirsiniz\n"
			" 2- İki elementin yerleri değiştirildiğinde tüm satırlar birer yukarı kayar ve en alt satıra rastgele elementler eklenir"
		);
	puts(	" 3- En üst satıra herhangi bir element ulaştığında oyun direkt sona erer\n"
			" 4- Bir element grubunu patlatabilmek için en az 3 tane yan yana veya alt alta aynı elementten bulunmalıdır\n"
			" 5- Eğer hem dikey hem yatay patlama olabiliyorsa oyun en fazla element patlayabilecek olanı seçecektir\n"
			" 6- Eğer dikey ve yatay patlayabilecek element sayısı eşitse oyun yatay olanı seçer"
		);
	puts(	" 7- Bir element grubunu patlatmaya karar verdiğinizde bu grubun en üst veya en sol elementinin koordinatını girmeniz gerekmektedir\n"
			" 8- Yer değiştirme veya patlama işlemi seçildikten sonra koordinat yerine rakam olmayan bir karakter girmek işlemi iptal etmeyi sağlar\n"
			"\n"
			"Oyun Modları: \n"
			" 1- Normal Mod:\n"
			"  Bu modda oyun matrisinin alttan n/2 kadar satırı rastgele elementlerle doldurulur.\n"
			"  Her hamleden sonra oyun ekranı temizlenir"
		);
	puts(	" 2- Kontrol Mod:\n"
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

/* terminali temizlemek için önişlemci komutları ile işletim sistemi tespit edilir */
/* eğer herhangi geçerli bir sistem tespit edilmezse 1000 kere \n basılır (işlevsel) */
void clear_screen()
{
	/* https://stackoverflow.com/a/5920028 */
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64) /* windows 32-bit veya 64-bit */
		system("cls");
	#elif defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) /* freedos veya dos-like (watcom) */
		system("cls");
	#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__) /* unix like */
		system("clear");
	#else /* herhangi platform özelinde fonksiyon kullanmadan kaba yöntemle temizle */
		int i = 0;
		for(i = 0; i < 1000; i++)
		{
			printf("\n");
		}
	#endif
}


/* tablo yaparken |-+ karakterleri yerine direkt tablo karakterleri kullanılabilirdi ancak kodun taşınabilirliği azalırdı */
/* Çünkü modern sistemlerde kullanılan UNICODE'e göre tablo karakterleri ile eski tip (ANSI) tablo karakterlerinin karakter kodları farklı */
/* bu da UNICODE kullanan terminaller ile ANSI kullanan terminaller arasında taşınabilirliği azaltırdı */
void print_matrix(GameMatrix gameMatrix, MatrixSize matrixSize) /* genel bir fonksiyon değil, iyi bir görünüm için en fazla 20 sütun destekler */
{
	int i = 0, j = 0;
	
	/* sütunun onlar basamağı yazılır */
	printf("\n  |%-.*s|\n  ", matrixSize.m*2-1, "0                 1                   2"); /* sütun sayısında göre 3. parametredeki string'in belli bir kısmı yazılır */
	/* sütun numarasının onlar basamağı yazıldı */
	
	/* sütun numarasının birler basamağı yazılır */
	for(j = 0; j < matrixSize.m; j++)
	{
		printf("|%d", (j+1)%10);
	}
	printf("|\n");
	/* sütun numarasının birler basamağı yazıldı */
	
	for(j = 0; j < matrixSize.m*2+3; j++) /* satırların en üstüne tire işaretleri konulur */
	{
		printf("-");
	}
	printf("\n");
	
	
	for(i = 0; i < matrixSize.n; i++)
	{
		printf("%02d", i+1); /* her satırın başında satır numarası yaz */
		for(j = 0; j < matrixSize.m; j++)
		{
			printf("|%c", (gameMatrix[i][j] == '\0') ? ' ' : gameMatrix[i][j]); /* tablo karakterini ve oyun karakterini yaz */
		}
		printf("|\n"); /* tablo karakterinin en sonuncusunu koy */
		for(j = 0; j < matrixSize.m*2+3; j++) /* yazılan satırın altına tire işaterlerini koy */
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
void print_status(GameMatrix gameMatrix, MatrixSize matrixSize, GameStatus gameStatus)
{
	if(gameStatus.gameMode == GAME_MODE_NORMAL) /* eğer oyun modu ise ekranı temizle */
	{
		clear_screen();
	}
	/* durumu ekrana yaz */
	printf("\nToplam yer değişikliği: %d\nToplam patlatılan element: %d\n", gameStatus.movementCount, gameStatus.explosionCount);
	print_matrix(gameMatrix, matrixSize);
}

/* 1. parametredeki matrisin tüm değerlerini 2. parametredeki adresde bulunan MatrixSize yapısına göre '\0' yapar */
/* 2. parametrenin pointer olmasının sebebi tek adres ile büyük bir yapıya erişebilmek (tüm elemanları kopyalamak yerine) */
void clear_matrix(GameMatrix gameMatrix, MatrixSize matrixSize)
{
	int i = 0, j = 0;
	for(i = 0; i < MIDDLE_ROW(matrixSize.n); i++)
	{
		for(j = 0; j < matrixSize.m; j++)
		{
			gameMatrix[i][j] = '\0';
		}
	}
}

/* oyunun en başnda kullanıcıdan matrisin boyutunu almak için fonksiyon */
MatrixSize get_matrix_size(void)
{
	int is_valid = 0, items_read = 0;
	MatrixSize matrixSize = {0, 0};
	do
	{
		is_valid = 1; /* Girdinin en başta doğru olduğu varsayılır */
		printf("Oyun alanının boyutlarını (N M) giriniz [%d-%d]: ", MIN_GAME_MATRIX_SIZE, MAX_GAME_MATRIX_SIZE);
		items_read = scanf("%d %d", &matrixSize.n, &matrixSize.m);
		clear_stdin();
		if(items_read != 2)
		{
			printf("Geçersiz girdi, bu girdi için 2 tane [%d-%d] arası tamsayı girmelisiniz!\n", MIN_GAME_MATRIX_SIZE, MAX_GAME_MATRIX_SIZE);
			is_valid = 0;
		}
		else /* girdi sayısı doğru bu yüzden n ve m'nin değerlerini kontrol edebiliriz */
		{
			if(matrixSize.n < MIN_GAME_MATRIX_SIZE || matrixSize.n > MAX_GAME_MATRIX_SIZE)
			{
				printf("N=%d değeri geçersiz aralıktadır!\n", matrixSize.n);
				is_valid = 0;
			}
			if(matrixSize.m < MIN_GAME_MATRIX_SIZE || matrixSize.m > MAX_GAME_MATRIX_SIZE)
			{
				printf("M=%d değeri geçersiz aralıktadır!\n", matrixSize.m);
				is_valid = 0;
			}
		}
		
		if(is_valid == 0) /* girdi hatalıysa kullanıcıyı bekle */
		{
			pause();
		}
		
	}while(is_valid == 0);
	return matrixSize;
}

/* kullanıcıdan oyun modu istenir */
GameMode get_game_mode(void)
{
	int is_valid = 0, items_read = 0, gameMode = 0; /* return'de cast edilir */
	do
	{
		is_valid = 1;
		printf("Oyun modu için %d, kontrol modu için %d giriniz: ", GAME_MODE_NORMAL, GAME_MODE_CONTROL);
		items_read = scanf("%d", &gameMode);
		clear_stdin();
		if(items_read != 1)
		{
			printf("Geçersiz girdi, bu girdi için %d veya %d tamsayı değerlerini girmelisiniz!\n", GAME_MODE_NORMAL, GAME_MODE_CONTROL);
			is_valid = 0;
			pause();
		}
		else if(gameMode != GAME_MODE_NORMAL && gameMode != GAME_MODE_CONTROL)
		{
			printf("Geçersiz mod, geçerli modlar: %d ve %d\n", GAME_MODE_NORMAL, GAME_MODE_CONTROL);
			is_valid = 0;
			pause();
		}
	}while(is_valid == 0);
	return (GameMode)gameMode;
}

GameOperation get_game_operation()
{
	int is_valid = 0, items_read = 0, gameOperation = 0; /* return'da cast edilir */
	do
	{
		is_valid = 1;
		printf( "Yer değişikliği için %d\n"
				"Patlama için %d\n"
				"Oyundan çıkmak için %d giriniz: ", GAME_OPERATION_MOVE, GAME_OPERATION_EXPLODE, GAME_OPERATION_EXIT);
		items_read = scanf("%d", &gameOperation);
		clear_stdin();
		if(items_read != 1)
		{
			printf("Geçersiz girdi, bu girdi için %d, %d veya %d tamsayı değerlerini girmelisiniz!\n", GAME_OPERATION_MOVE, GAME_OPERATION_EXPLODE, GAME_OPERATION_EXIT);
			is_valid = 0;
		}
		else if(gameOperation != GAME_OPERATION_MOVE && gameOperation != GAME_OPERATION_EXPLODE && gameOperation != GAME_OPERATION_EXIT)
		{
			printf("Geçersiz işlem, geçerli işlemler: {%d, %d, %d}\n", GAME_OPERATION_MOVE, GAME_OPERATION_EXPLODE, GAME_OPERATION_EXIT);
			is_valid = 0;
		}
		
		if(is_valid == 0)
		{
			pause();
		}
	}while(is_valid == 0);
	return (GameOperation)gameOperation;
}

/* oyun moduna göre gameMatrix'i kullanıcıdan alır veya rastgele oluşturur */
void initialize_game_matrix(GameMatrix gameMatrix, MatrixSize matrixSize, GameMode gameMode)
{
	int is_valid = 0, i = 0, j = 0, k = 0;
	char ch = '\0';
	
	/* game_matrix'in boş kalması gereken kısımlarına '\0' değeri atanır */
	clear_matrix(gameMatrix, matrixSize);
	
	if(gameMode == GAME_MODE_NORMAL)
	{
		for(i = MIDDLE_ROW(matrixSize.n); i < matrixSize.n; i++) /* n-n/2 dedik eğer direkt n/2 deseydik "üstten n/2 boş bırak" demiş olurduk, bu da tek sayıda satır varsa bir satır daha fazla dolu olmasına sebep olurdu." */
		{
			for(j = 0; j < matrixSize.m; j++)
			{
				gameMatrix[i][j] = game_characters[rand() % GAME_CHARACTERS_LENGTH]; /* rastgele bir indisdeki karakteri alır */
			}
		}
	}
	else if(gameMode == GAME_MODE_CONTROL)
	{
		printf( "Kontrol modunu seçtiniz %d-%d satırlarının başlangıç durumlarını girmeniz gerekmektedir.\n"
				"Girebileceğiniz karakterler: [%.*s]\n", MIDDLE_ROW(matrixSize.n)+1, matrixSize.n, GAME_CHARACTERS_LENGTH, game_characters);
		for(i = MIDDLE_ROW(matrixSize.n); i < matrixSize.n; i++) /* n-n/2 dedik eğer direkt n/2 deseydik "üstten n/2 boş bırak" demiş olurduk, bu da tek sayıda satır varsa bir satır daha fazla dolu olmasına sebep olurdu." */
		{
			for(j = 0; j < matrixSize.m; j++)
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
				gameMatrix[i][j] = ch; /* girdi kontrolü yapıldı, atama yapılabilir */
			}
		}
	}
}

MovementCoords get_movement_coordinates(GameMatrix gameMatrix, MatrixSize matrixSize, GameStatus gameStatus)
{
	int is_valid = 0, items_read = 0;
	MovementCoords movementCoords = {{0, 0}, {0, 0}};
	/* Yerleri değişecek iki koordinat kullanıcıdan alınır */
	do
	{
		is_valid = 1; /* girdinin en başta doğru olduğu varsayıldı */
		printf("Yerlerini değiştirmek istediğiniz iki koordinatı giriniz (format: r1,c1 r2,c2): ");
		items_read = scanf("%d , %d %d , %d", &movementCoords.c1.i, &movementCoords.c1.j, &movementCoords.c2.i, &movementCoords.c2.j);
		clear_stdin();
		if(items_read != 4)
		{
			printf("Yer değiştirme işlemi iptal edildi\n");
			pause();
		}
		else
		{
			/* burada hataları else if.. diye değil de direkt if ile kontrol etmemizin sebebi */
			/* eğer birden fazla yanlış koordinat varsa hepsini birden kullanıcıya haber etsin için */
			if(movementCoords.c1.i < MIN_GAME_MATRIX_POSITON || movementCoords.c1.i > MAX_GAME_MATRIX_POSITON)
			{
				is_valid = 0;
				printf("r1=%d değeri geçersiz aralıktadır!\n", movementCoords.c1.i);
			}
			if(movementCoords.c1.j < MIN_GAME_MATRIX_POSITON || movementCoords.c1.j > MAX_GAME_MATRIX_POSITON)
			{
				is_valid = 0;
				printf("c1=%d değeri geçersiz aralıktadır!\n", movementCoords.c1.j);
			}
			if(movementCoords.c2.i < MIN_GAME_MATRIX_POSITON || movementCoords.c2.i > MAX_GAME_MATRIX_POSITON)
			{
				is_valid = 0;
				printf("r2=%d değeri geçersiz aralıktadır!\n", movementCoords.c2.i);
			}
			if(movementCoords.c2.j < MIN_GAME_MATRIX_POSITON || movementCoords.c2.j > MAX_GAME_MATRIX_POSITON)
			{
				is_valid = 0;
				printf("c2=%d değeri geçersiz aralıktadır!\n", movementCoords.c2.j);
			}
			if(is_valid == 1) /* koordinatlar geçerli aralıkta, şimdi koordinatların yer değiştirip değiştiremeyeceğini kontrol et */
			{
				if(gameMatrix[movementCoords.c1.i-1][movementCoords.c1.j-1] == '\0')
				{
					is_valid = 0;
					printf("Girdiğiniz 1. koordinatlarda (%d,%d) herhangi bir karakter bulunmamaktadır!\n", movementCoords.c1.i, movementCoords.c1.j);
				}
				if(gameMatrix[movementCoords.c2.i-1][movementCoords.c2.j-1] == '\0')
				{
					is_valid = 0;
					printf("Girdiğiniz 2. koordinatlarda (%d,%d) herhangi bir karakter bulunmamaktadır!\n", movementCoords.c2.i, movementCoords.c2.j);
				}
				if(movementCoords.c1.i != movementCoords.c2.i || abs(movementCoords.c1.j-movementCoords.c2.j) != 1) /* koordinatlar yan yana, sadece yan yana koordinatlar yer değiştirebilir*/
				{
					is_valid = 0;
					printf("(%d,%d) koordinatları ile (%d,%d) koordinatları yan yana değil!\n", movementCoords.c1.i, movementCoords.c1.j, movementCoords.c2.i, movementCoords.c2.j);
				}
				/*else if(abs(movementCoords.c1.i-movementCoords.c2.i)+abs(movementCoords.c1.j-movementCoords.c2.j) != 1) // koordinatlar yan yana veya alt alta değil, sadece yan yana veya alt alta koordinatlar yer değiştirebilir
				{
					is_valid = 0;
					printf("(%d,%d) koordinatları ile (%d,%d) koordinatları yan yana veya alt alta değil!\n", movementCoords.c1.i, movementCoords.c1.j, movementCoords.c2.i, movementCoords.c2.j);
				}*/
			}
		}
		if(is_valid == 0) /* hata oldu, kullanıcıya bilgi verildi, bekle */
		{
			pause();
			print_status(gameMatrix, matrixSize, gameStatus);
		}
	}while(is_valid == 0);
	movementCoords.c1.i--, movementCoords.c1.j--, movementCoords.c2.i--, movementCoords.c2.j--;
	return movementCoords;
}

/* geri dönüş değeri olarak oyunun bitip bitmediğini döner (0 bitmedi, 1 bitti) */
int do_movement(GameMatrix gameMatrix, MatrixSize matrixSize, GameStatus gameStatus, MovementCoords movementCoords)
{
	int ended = 0, temp = 0, i = 0, j = 0;
	char ch = '\0';
	
	ch = gameMatrix[movementCoords.c1.i][movementCoords.c1.j];
	gameMatrix[movementCoords.c1.i][movementCoords.c1.j] = gameMatrix[movementCoords.c2.i][movementCoords.c2.j];
	gameMatrix[movementCoords.c2.i][movementCoords.c2.j] = ch;
	
	/* Burada yer değiştirme işlemi yapıldıktan sonra, ama aşağı yeni satır eklemeden önce bizden matrisi yazdırmamız istendiği için */
	print_status(gameMatrix, matrixSize, gameStatus);
	printf("Yer değiştirildi. Yeni satır ekleniyor\n");
	pause();
	
	
	
	for(j = 0; j < matrixSize.m; j++) /* matrisin en altına yeni satırı ekleme döngüsü, her sütuna tek tek eklenir */
	{
		/* matrisin en alt satırından üst satırlara doğru kaydırma işlemi yapılır. */
		/* en üstten başlamak daha az değişken kullanmamızı sağlardı ancak bu sefer önce boş olmayan satırı bulmamız gerekirdi (veya boş yere işlem gücü harcardık boş satırları kaydırarak) */
		temp = gameMatrix[matrixSize.n-1][j];
		i = matrixSize.n-2;
		while(i >= 0 && temp != '\0') /* temp != '\0' kontrolü ile gereksiz kaydırma işlemlerinden kaçınmış olduk */
		{
			int old_val = gameMatrix[i][j];
			gameMatrix[i][j] = temp;
			temp = old_val;
			i--;
		}
		/* sütundaki her satır bir üste kaydırıldı, şimdi rastgele yeni bir karakteri en alta yaz */
		gameMatrix[matrixSize.n-1][j] = game_characters[rand() % GAME_CHARACTERS_LENGTH];
		/* en üst satırı kontrol et, eğer bir taş varsa ended = 1 */
		if(gameMatrix[0][j] != '\0')
		{
			ended = 1; /* yani kaybettin */
		}
	}
	/* kaydırma işlemi bitti */
	/* proje ön bilgisinde "Oyun tüm alan doluncaya" kadar denmiş ancak örnekte üst satırda birkaç tane sütun dolduğunda */
	/* oyun sonlandırılmış. Ben de bana daha mantıklı gelen üst satırda herhangi dolu bir kutu varsa oyun bitsin şeklinde yaptım*/
	if(ended == 1) /* en üst satırda 0 olmayan eleman var */
	{
		print_status(gameMatrix, matrixSize, gameStatus);
		printf("Oyun en üst satırda eleman bulunduğu için sonlandırılmıştır.");		
	}
	return ended;
}

/* Eğer işlem kullanıcı tarafından iptal edilmek istenirse (rakam olmayan bir karaktere basılırsa) koordinat olarak {-1, -1} döner. (normal şartlarda olmayacak bir koordinat) */
Coord get_explosion_coordinate(GameMatrix gameMatrix, MatrixSize matrixSize, GameStatus gameStatus)
{
	int is_valid = 0, items_read = 0;
	Coord coord = {0, 0};
	do
	{
		is_valid = 1; /* girdinin en başta doğru olduğu varsayıldı */
		printf("Patlatmak istediğiniz bloğun sol veya üst koordinatını giriniz (format: r,c): ");
		items_read = scanf("%d , %d", &coord.i, &coord.j);
		clear_stdin();
		if(items_read != 2)
		{
			printf("Patlatma işlemi iptal edildi\n");
			pause();
		}
		else
		{
			if(coord.i < MIN_GAME_MATRIX_POSITON || coord.i > MAX_GAME_MATRIX_POSITON)
			{
				is_valid = 0;
				printf("r=%d değeri geçersiz aralıktadır!\n", coord.i);
			}
			if(coord.j < MIN_GAME_MATRIX_POSITON || coord.j > MAX_GAME_MATRIX_POSITON)
			{
				is_valid = 0;
				printf("c=%d değeri geçersiz aralıktadır!\n", coord.j);
			}
			/* önce is_valid kontrol ettik ki eğer sınır dışında ise rastgele bellek adresine erişmeye çalışmasın */
			if(is_valid == 1 && gameMatrix[coord.i-1][coord.j-1] == '\0') /* boş kutu patlatılamaz */
			{
				is_valid = 0;
				printf("Girdiğiniz koordinatta (%d,%d) herhangi bir karakter bulunmamaktadır!\n", coord.i, coord.j);
			}
		}
		if(is_valid == 0) /* hata oldu, kullanıcıya bilgi ver */
		{
			pause();
			print_status(gameMatrix, matrixSize, gameStatus);
		}
	}while(is_valid == 0);
	coord.i--, coord.j--;
	return coord;
}

/* geri dönüş değeri olarak patlatılan taş sayısını döner */
int do_explosion(GameMatrix gameMatrix, MatrixSize matrixSize, Coord coord)
{
	int i = 0, j = 0, k = 0;
	int explosionCount = 0;
	int right_length = 1, down_length = 1; /* hangisi daha fazlaysa o kullanılacak, 1'den başlattık çünkü kendisi de arıyor */
	/* Şimdi bu koordinatta patlama olabilir mi onu kontrol etmeliyiz */
	/* proje kapsamında patlatmak için elemanın sol üst köşe olduğu belirtildiği için */
	/* sadece sağa ve aşağı doğru kontrol etmek yeterlidir. */
	
	
	/* mesafelerin tespit edilmesi */
	for(j = coord.j+1; j < matrixSize.m && gameMatrix[coord.i][j-1] == gameMatrix[coord.i][j]; j++) /* sağa doğru */
	{
		right_length++;
	}
	for(i = coord.i+1; i < matrixSize.n && gameMatrix[i-1][coord.j] == gameMatrix[i][coord.j]; i++) /* aşağı doğru */
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
			for(i = coord.i-1; i >= 0 && gameMatrix[i][coord.j+j] != '\0'; i--) /* sütunun dolu olduğu elemanları birer birer aşağı ötele */
			{
				gameMatrix[i+1][coord.j+j] = gameMatrix[i][coord.j+j]; /* satırı birer birer aşağı kaydır */
			}
			gameMatrix[i+1][coord.j+j] = '\0'; /* aşağı düşmeden önce en üstte bulunan satır artık bir aşağıda, bunu 0 yapmalıyız. */
		}
		explosionCount += right_length; /* patlama gerçekleşti */
	}
	else /* Burada bir şeyi kontrol etmemize gerek yok zaten ilk iki if'in olmama durumunda aşağı uzunluk sağ uzunluktan büyük ve aşağı uzunluk yeterli bir uzunluğu sahip */
	{
		/* bu dögüde k indisi patlama olan bölgenin üstündeki elemanları, i indisi patlama olan bölgeleri temsil eder */
		k = coord.i-1;
		for(i = coord.i-1+down_length; k >= 0 && gameMatrix[k][coord.j] != '\0'; i--, k--) /* TODO: k >= 0 gerekli değil */
		{
			gameMatrix[i][coord.j] = gameMatrix[k][coord.j]; /* üstteki satırı down_length kadar aşağı al */
		}
		while(i > k) /* aşağı düşen satırları '\0' değeri atadık */
		{
			gameMatrix[i][coord.j] = '\0';
			i--;
		}
		explosionCount += down_length; /* patlama gerçekleşti */
	}
	/* kaydırma işlemi bitti */
	return explosionCount;
}

int main()
{
	int ended = 0;
	MatrixSize matrixSize = {0, 0};
	GameMatrix gameMatrix;
	GameStatus gameStatus = {GAME_MODE_UNKNOWN, 0, 0};
	GameOperation gameOperation = GAME_OPERATION_UNKNOWN;
	
	
	srand(time(NULL)); /* rastgele sayı üretmek için zamanı kullandık. basit bir oyun için yeterli bir "rastgelelik" sağlar */
	
	print_splash_screen();
	
	/* Kullanıcıdan matris boyutunun alınması ve kontrol edilmesi, geçersizse gerekli mesajların yazılması */
	matrixSize = get_matrix_size();
	/* Kullanıcıdan matrisin boyunu alma işlemi bitti */
	
	
	/* Kullanıcıdan oyun modunun alınması ve kontrol edilmesi, gerekirse hata mesajlarının yazılması */
	gameStatus.gameMode = get_game_mode();
	/* Kullanıcıdan oyun modunu alma bitti */
	
	
	
	
	/* Modlara göre matrisi oluştur veya iste */
	initialize_game_matrix(gameMatrix, matrixSize, gameStatus.gameMode);
	/* Modlara göre matrisi oluşturma işlemi bitti. */
	
	
	/* Oynun asıl döngüsü */
	do
	{
		print_status(gameMatrix, matrixSize, gameStatus);
		
		/* Kullanıcıdan yapılacak işlemin alınması ve kontrol edilmesi, gekerirse hata mesajlarının yazılması */
		gameOperation = get_game_operation();
		/* Kullanıcıdan yapılacak işlemi alma bitti */
		
		if(gameOperation == GAME_OPERATION_EXIT) /* Oyundan çıkılmak istendiğinde */
		{
			printf("Oyun kullanıcı isteği ile sonlandırılmıştır.\n");
			ended = 1;
		}
		else if(gameOperation == GAME_OPERATION_MOVE) /* İki karakterin yerini değiştirmek istediğinde */
		{
			MovementCoords movementCoords = get_movement_coordinates(gameMatrix, matrixSize, gameStatus);
			if(movementCoords.c1.i != -1 && movementCoords.c1.j != -1 && movementCoords.c2.i != -1 && movementCoords.c2.j != -1) /* işlem iptal edilmedi */
			{
				ended = do_movement(gameMatrix, matrixSize, gameStatus, movementCoords);
				gameStatus.movementCount++;
			}
		}
		/* patlatmak */
		else /* operation == GAME_OPERATION_EXPLODE, ancak başka bir şey olamayacağı için direkt else dedik */
		{
			Coord coord = get_explosion_coordinate(gameMatrix, matrixSize, gameStatus);
			if(coord.i != -1 && coord.j != -1)
			{
				gameStatus.explosionCount += do_explosion(gameMatrix, matrixSize, coord);
			}
		}
	}while(ended == 0);
	
	return 0;
}