#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <time.h>

extern "C" 
{
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540
#define MAP_HEIGHT 9000
#define MAP_WIDTH 960
#define NUMBER_OF_SAVES 9
#define LENGTH_OF_GAMESAVE_NAME 24
#define LENGTH_OF_GAMESAVE_DATAFORMAT_NAME 20
#define CAR_HEIGHT 60
#define CAR_WIDTH 40
#define ROAD_SPEED 0.5
#define ENEMY_SPAWN 5200
#define FRIEND_SPAWN 200
#define CAR_SPAWN 100

//sprawdzenie koloru piksela
SDL_Color GetPixelColor(SDL_Surface* surface, int x, int y)
{
	Uint8 bpp = surface->format->BytesPerPixel;

	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

	Uint32 pixeldata = *(Uint32*)p;

	SDL_Color color = { 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE };

	SDL_GetRGB(pixeldata, surface->format, &color.r, &color.g, &color.b);

	return color;
}

//narysowanie napisu na powierzchni screen, zaczynaj�c od punktu (x, y), charset to bitmapa 128x128 zawieraj�ca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

//narysowanie na powierzchni screen powierzchni sprite w punkcie (x, y); (x, y) to punkt �rodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

//rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

//rysowanie linii o d�ugo�ci l w pionie (gdy dx = 0, dy = 1) b�d� poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};

//rysowanie prostok�ta o d�ugo�ci bok�w l i k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

//tworzenie nazwy pliku z zapisem
void fileName(char gamesave[80]) {

	//tworzenie nazwy pliku z zapisem
	time_t czas;
	struct tm* data;
	char czas_char[80];

	time(&czas);
	data = localtime(&czas);

	strftime(czas_char, 80, "%F#%H-%M-%S.txt", data);
	sprintf(gamesave, "%s ", czas_char);
	gamesave[LENGTH_OF_GAMESAVE_NAME-1] = '\0';
}

//zapisywanie gry
void saveGame(float roadspeed, int score, double distance, double worldTime, int position, float enemyspeed, char gamesaves[10][24], int &number_of_save)
{	
	//usuwanie starego zapisu gry
	char file_to_delete[80] = {};

   for (int y = 0; y < LENGTH_OF_GAMESAVE_NAME; y++)
	{
		file_to_delete[y] = gamesaves[number_of_save][y];
	}
	
   remove(file_to_delete);
	
	//tworzenie pliku z zapisem
	char gamesave[80] = {};
	char c = '#';

	fileName(gamesave);

	 for (int y = 0; y < LENGTH_OF_GAMESAVE_NAME; y++)
	 {
		 gamesaves[number_of_save][y] = gamesave[y];
	 }

	FILE* file = fopen(gamesave, "w");

	if (file != NULL)
	{
		fprintf(file, "%d", score);
		fprintf(file, "%c", c);
		fprintf(file, "%f", roadspeed);
		fprintf(file, "%c", c);
		fprintf(file, "%f", distance);
		fprintf(file, "%c", c);
		fprintf(file, "%f", worldTime);
		fprintf(file, "%c", c);
		fprintf(file, "%d", position);
		fprintf(file, "%c", c);
		fprintf(file, "%f", enemyspeed);
		
		fclose(file);
	}

	//zapisywanie nazwy zapisu do pliku z nazwami zapis�w
	char saves_names[80] = { "saves_names.txt" };

	FILE* file2 = fopen(saves_names, "w");

	number_of_save++;
	
	//zerowanie i, je�li przekroczono ilo�� dozwolonych zapis�w
	if (number_of_save == NUMBER_OF_SAVES) number_of_save = 0;

	if (file2 != NULL)
	{
		fprintf(file, "%d\n", number_of_save);
		for (int z = 0; z < NUMBER_OF_SAVES; z++)
		{
			if (gamesaves[z][0] != '�')
			{
			
			fprintf(file, "%s\n", gamesaves[z]);
		}
	}

		fclose(file);
	}
}

//�adowanie gry
void loadGame(float& roadspeed, int &score, float& distance, float& worldTime, int& position, float& enemyspeed, char gamesave[80], int &number_of_save)
{
	//odczytywanie zapisu gry
	char c = {};

	FILE* file = fopen(gamesave, "r");

	if (file != NULL)
	{
		fscanf(file, "%d", &score);
		fscanf(file, "%c", &c);
		fscanf(file, "%f", &roadspeed);
		fscanf(file, "%c", &c);
		fscanf(file, "%f", &distance);
		fscanf(file, "%c", &c);
		fscanf(file, "%f", &worldTime);
		fscanf(file, "%c", &c);
		fscanf(file, "%d", &position);
		fscanf(file, "%c", &c);
		fscanf(file, "%f", &enemyspeed);

		fclose(file);
	}
}

//�adowanie konkretnego zapisu gry
void loadingButtonPressed(int w, char gamesaves[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_NAME], int &score, float &roadspeed, float &distance, int &position, float &worldTime, int &pause, float &enemyspeed, int &t2, int&t3, int&t4, int &game_saves_screen, int &number_of_save)
{
	//odczytywanie nazwy pliku z zapisem
	char gamesave[80] = {};
	for (int y = 0; y < LENGTH_OF_GAMESAVE_NAME; y++)
	{
		gamesave[y] = gamesaves[w][y];
	}

	//�adowanie zapiu gry
	loadGame(roadspeed, score, distance, worldTime, position, enemyspeed, gamesave, number_of_save);

	pause = 0;
	t2 = SDL_GetTicks();
	t4 = t2 - t3;
	game_saves_screen = 0;
}

//tworzenie tablicy z nazwami zapis�w
void dataformatForGameSaves(char gamesaves_dataformat[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_DATAFORMAT_NAME], char gamesaves[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_NAME]) {
	for (int i = 0; i < NUMBER_OF_SAVES; i++)
	{
		for (int y = 0; y < (LENGTH_OF_GAMESAVE_DATAFORMAT_NAME-1); y++)
		{
			if (gamesaves[i][y] == '#') gamesaves_dataformat[i][y] = ' ';
			else if (gamesaves[i][y] == '-' && y>9) gamesaves_dataformat[i][y] = ':';
			else gamesaves_dataformat[i][y] = gamesaves[i][y];
		}
		gamesaves_dataformat[i][LENGTH_OF_GAMESAVE_DATAFORMAT_NAME-1] = '\0';
	}
}

//towrzenie tablicy z nazwami plik�w z zapisami
void createGameSavesFromeFile(char gamesaves[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_NAME], int &number_of_save) {
	char saves_names[80] = { "saves_names.txt" };

	FILE* file = fopen(saves_names, "r");

	if (file != NULL)
	{
		fscanf(file, "%d", &number_of_save);
		for (int z = 0; z < NUMBER_OF_SAVES; z++)
		{
			fscanf(file, "%s", gamesaves[z]);
		}

		fclose(file);
	}
}

//zi�kszanie zmiennych, je�li nie ma pauzy
void increaseVariables(int& score, float& roadspeed, float& carspeed, float& enemyspeed, float& distance, int &timestop, int minuspoints) {
	int enemyspeed_int = (int)enemyspeed;
	if (enemyspeed_int > ENEMY_SPAWN)
	{
		enemyspeed = enemyspeed + 0.9 - carspeed;
	}
	else enemyspeed = enemyspeed + ROAD_SPEED+0.1+ carspeed;
	roadspeed = roadspeed + ROAD_SPEED + carspeed;
	if (timestop == 0)
	{
		score = distance * 10- minuspoints;
	}
	else timestop--;
}

//sprawdzanie czy samochodzik nie zgin��
void ifDeath(float &worldTime, float &roadspeed, int &position, int &score, float &distance, float &enemyspeed, int &timestop, int& minuspoints)
{
	worldTime = 0;
	score = 0;
	roadspeed = 0;
	position = SCREEN_WIDTH / 2;
	distance = 0;
	enemyspeed = 0;
	timestop = 0;
	minuspoints = 0;
}

//rysowanie drogi z samochodami
void drawRoadWithCars(SDL_Surface* screen, SDL_Surface* car, SDL_Surface* enemy, SDL_Surface* notenemy, SDL_Surface* road, float roadspeed, int position, float enemyspeed) {
	//rysowanie planszy
	DrawSurface(screen, road, SCREEN_WIDTH / 2, SCREEN_HEIGHT/2  - MAP_HEIGHT / 2+300 + roadspeed);

	//rysowanie samochodziku
	DrawSurface(screen, car, position, SCREEN_HEIGHT / 2 + CAR_SPAWN);

	//rysowanie wroga
	DrawSurface(screen, enemy, SCREEN_WIDTH / 2, SCREEN_HEIGHT + ENEMY_SPAWN - enemyspeed);

	//rysowanie przyjaciela
	DrawSurface(screen, notenemy, SCREEN_WIDTH / 2, SCREEN_HEIGHT + FRIEND_SPAWN - enemyspeed);
}

//rysowanie okna z informacjami
void drawInformationWindows(SDL_Surface* screen, SDL_Surface* charset, int score, float worldTime, int marine_color, float distance)
{
	char text[100] = {};

	//rysowanie gornego okienka
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 70, marine_color, marine_color);

	//wypisywanie imienie, nazwiska i nr. indeksu
	sprintf(text, "Franciszek Gwarek 193192");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

	//wypisywanie czasu i wyniku
	sprintf(text, "Czas = %.0lf s, Wynik = %d, Dystans = %.0lf px", worldTime, score, distance);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

	//wypisywanie dzialania klawiszy
	sprintf(text, "Esc - wyjscie, n - nowa gra, p - pauza, s - zapisz gre, l - wczytaj gre");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);

	sprintf(text, "\032 / \033 - skrecanie \030 - przyspieszenie");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 60, text, charset);

	//rysowanie okna z numerami funkcjonalnosci
	DrawRectangle(screen, 600, SCREEN_HEIGHT - 31, SCREEN_WIDTH - 600, 30, marine_color, marine_color);
	sprintf(text, "Funkcjonalnosci: a-j");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2 + 300, SCREEN_HEIGHT - 20, text, charset);
}

//liczenie czasu i dystansu
void timeOperations(int &t1, int& t2, int& t3, int& t4, double &delta, float &worldTime, float &distance, float &carspeed) {
	t2 = SDL_GetTicks();
	if (t3 != 0)
		t2 = t2 - t4;
	// w tym momencie t2-t1 to czas w milisekundach,
	// jaki uplyna� od ostatniego narysowania ekranu
	// delta to ten sam czas w sekundach
	delta = (t2 - t1) * 0.001;
	t1 = t2;

	worldTime += delta;
	distance += (carspeed + ROAD_SPEED) * delta;
}

//ifMPBisNULL
int ifBMPisNULL(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 1;
}

//nie dzia�a!!!
int creatBMPs(SDL_Surface* screen, SDL_Surface* charset, SDL_Surface* car, SDL_Surface* enemy, SDL_Surface* road, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
	};

	SDL_SetColorKey(charset, true, 0x000000);

	//wczytywanie saochodziku
	car = SDL_LoadBMP("./car.bmp");
	if (car == NULL) {
		printf("SDL_LoadBMP(car.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
	};

	//wczytywanie drogi
	road = SDL_LoadBMP("./road.bmp");
	if (car == NULL) {
		printf("SDL_LoadBMP(road.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
	};

	//wczytywanie wroga
	enemy = SDL_LoadBMP("./enemy.bmp");
	if (enemy == NULL) {
		printf("SDL_LoadBMP(enemy.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
	};
}

//tworzenie kolor�w
void createColors(int &black_color, int &marine_color, int &darkblue_color, SDL_Surface* screen) {
	black_color = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	marine_color = SDL_MapRGB(screen->format, 0x00, 0x66, 0x66);
    darkblue_color = SDL_MapRGB(screen->format, 0x00, 0x33, 0x33);
}

//kolor trawy i po bokach
void deathColor(SDL_Color &grasscolor, SDL_Surface* road, SDL_Surface* screen, int &grasscolor_int, int &colorright_int, int &colorleft_int) {
	grasscolor = GetPixelColor(road, 0, 0);
	grasscolor_int = SDL_MapRGB(screen->format, grasscolor.r, grasscolor.g, grasscolor.b);
}

//rysowanie okna z zapisami gier
void gamesSavesScreen(SDL_Surface* screen, SDL_Surface* charset,  int darkblue_color, char gamesaves[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_NAME], char gamesaves_dataformat[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_DATAFORMAT_NAME]) {
	char text[100] = {};
	int gamesaves_position = 100;
	int k = 1;
	DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, darkblue_color, darkblue_color);
	sprintf(text, "Wcisnij cyfre, aby wczytac dany zapis gry");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, gamesaves_position - 50, text, charset);
	for (int y = 0; y < NUMBER_OF_SAVES; y++)
	{

		if (gamesaves[y][0] != '�')
		{
			sprintf(text, "%d. %s", k, gamesaves_dataformat[y]);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, gamesaves_position, text, charset);
			gamesaves_position += 20;
			k++;
		}
	}
}

//sprawdzanie koloru pikseli na kt�re wje�d�a samochodzik
void checkColors(SDL_Surface* screen, SDL_Surface* road, SDL_Color& colorright, SDL_Color& colorleft, int &colorright_int, int &colorleft_int, int position, int roadspeed) {
	colorright = GetPixelColor(road, position + CAR_WIDTH / 2, MAP_HEIGHT - 180 - roadspeed);
	colorright_int = SDL_MapRGB(screen->format, colorright.r, colorright.g, colorright.b);
	colorleft = GetPixelColor(road, position - CAR_WIDTH / 2, MAP_HEIGHT - 180 - roadspeed);
	colorleft_int = SDL_MapRGB(screen->format, colorleft.r, colorleft.g, colorleft.b);
}

//je�li uderzono przyjacielski samoch�d
void ifFriendisHit(float &enemyspeed, int &timestop, int &minuspoints) {
	minuspoints += 20;
	enemyspeed = -MAP_HEIGHT;
	timestop = 1000;
}

#ifdef __cplusplus
extern "C"
#endif

//funkcja g��wna
int main(int argc, char **argv) 
  {
    //tworzenie zmiennych
	int t1 = 0, t2 = 0, t3 = 0, t4 = 0, minuspoints = 0, game_saves_screen = 0, score = 0, position = SCREEN_WIDTH / 2, pause = 0, enemyposition = 0, friendposition = 0, timestop = 0, quit = 0, frames = 0, rc = 0;
	double delta=0, fpsTimer=0, fps=0;
	float worldTime=0, distance=0;
	int distance_int=0;
	float roadspeed = 0, enemyspeed = 0, carspeed = 0;
	int number_of_save = 0;
	SDL_Event event;
	SDL_Surface *screen, *charset, *car, *enemy, *road, *notenemy;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Color grasscolor, colorright, colorleft;

	// okno konsoli nie jest widoczne, je�eli chcemy zobaczy� komunikaty wypisywane printf trzeba w opcjach: project -> szablon2 properties -> Linker -> System -> Subsystem zmieni� na "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	//tryb pe�noekranowy
    //rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	//nazwa okna
	SDL_SetWindowTitle(window, "SpyHunter");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	//wy��czenie widoczno�ci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	//creatBMPs(screen, charset, car, enemy, road, scrtex, window, renderer);

	//wczytanie cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
		};

	SDL_SetColorKey(charset, true, 0x000000);

	//wczytywanie saochodziku
	car = SDL_LoadBMP("./car.bmp");
	if(car == NULL) {
		printf("SDL_LoadBMP(car.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
		};

	//wczytywanie drogi
	road = SDL_LoadBMP("./road.bmp");
	if (car == NULL) {
		printf("SDL_LoadBMP(road.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
	};

	//wczytywanie wroga
	enemy = SDL_LoadBMP("./enemy.bmp");
	if (enemy == NULL) {
		printf("SDL_LoadBMP(enemy.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
	};

	//wczytywanie przyjaciela
	notenemy = SDL_LoadBMP("./friend.bmp");
	if (notenemy == NULL) {
		printf("SDL_LoadBMP(friend.bmp) error: %s\n", SDL_GetError());
		return ifBMPisNULL(screen, scrtex, window, renderer);
	};

	//tworzenie kolor�w
	int black_color, marine_color, darkblue_color;
	createColors(black_color, marine_color, darkblue_color, screen);

	//kolor trawy i kolory pobocza
	int grasscolor_int=0, colorright_int = 0, colorleft_int = 0;
	deathColor(grasscolor, road, screen, grasscolor_int, colorright_int, colorleft_int);
	
	//tablice z zapisami gier
	char gamesaves_dataformat[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_DATAFORMAT_NAME];
    char gamesaves[NUMBER_OF_SAVES][LENGTH_OF_GAMESAVE_NAME];

	//tworzenie tablicy z zapisami gier z pliku z nazwami zapis�w
	createGameSavesFromeFile(gamesaves, number_of_save);

	t1 = SDL_GetTicks();

	while (!quit) {
		
		//liczenie czasu i dystansu
		if (pause != 1)
		{
			timeOperations(t1, t2, t3, t4, delta, worldTime, distance, carspeed);
		}

		//wype�nianie ekranu kolorem
		SDL_FillRect(screen, NULL, black_color);

		//tworzenie tablicy z nazwami zapisanych gier
		dataformatForGameSaves(gamesaves_dataformat, gamesaves);

		distance_int = (int)distance;

		//zap�tlanie planszy
		if (distance_int%20== 0)
		{
			roadspeed= 0;
		}

		//rysowanie drogi i samochod�w
		drawRoadWithCars(screen, car, enemy, notenemy, road, roadspeed, position, enemyspeed);

		//sprawdzanie koloru pikseli na kt�re wje�d�a samochodzik
		checkColors(screen, road, colorright, colorleft, colorright_int, colorleft_int, position, roadspeed);

		//zwi�kszanie zmiannych, je�li nie ma pauzy
		if (pause != 1)
		{
			increaseVariables(score, roadspeed, carspeed, enemyspeed, distance, timestop, minuspoints);
		}

		enemyposition = SCREEN_HEIGHT + ENEMY_SPAWN - enemyspeed;

		//sprawdzanie czy nie wjechano na traw� lub wroga
		if (grasscolor_int == colorright_int || grasscolor_int == colorleft_int || (enemyposition == (SCREEN_HEIGHT / 2 + CAR_SPAWN) && position == (SCREEN_WIDTH / 2)))
		{
			ifDeath(worldTime, roadspeed, position, score, distance, enemyspeed, timestop, minuspoints);
		}

		//resetowanie pozycji wroga
		if (distance_int%25==0 && carspeed == 0)
		{
			enemyspeed = 0;
		}

		//sprawdzanie czy przyjaciel w nas nie wjecha�
		friendposition = SCREEN_HEIGHT + FRIEND_SPAWN - enemyspeed;

	    if (friendposition == (SCREEN_HEIGHT / 2 + CAR_SPAWN) && position == (SCREEN_WIDTH / 2))
			{
			ifFriendisHit(enemyspeed, timestop, minuspoints);
		    }

		//fpsy
		fpsTimer += delta;
		
		if (fpsTimer > 0.5) 
		{
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};
	
		//pomocnicze sprawdzanie koloru pixela na ktory wchodzimy
		//DrawRectangle(screen, 0, 0, 100, 100, colorleft_int, colorleft_int);
        //DrawRectangle(screen, SCREEN_WIDTH- 100, 0, 100, 100, colorleft_int, colorright_int);

		//rysowanie okna z informacjami
		drawInformationWindows(screen, charset, score, worldTime, marine_color, distance);

		//rysowanie okna z zapisami gier
		if (game_saves_screen == 1) 
		{
			gamesSavesScreen(screen, charset, darkblue_color, gamesaves, gamesaves_dataformat);	
		}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obs�uga zdarze� (o ile jakie� zasz�y)
		while(SDL_PollEvent(&event)) 
		{
			switch(event.type) 
			{
				//gdy wci�ni�to klawisz
			    case SDL_KEYDOWN:
					//wyj�cie z gry
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						quit = 1;
					}
					//skr�t w prawo
					else if (event.key.keysym.sym == SDLK_RIGHT) 
					{
						if (pause!=1) position += CAR_WIDTH / 2;
					}
					//skr�t w lewo
					else if (event.key.keysym.sym == SDLK_LEFT) 
					{
						if (pause != 1) position-= CAR_WIDTH / 2;
					}
					//nowa gra
					else if (event.key.keysym.sym == SDLK_n) {
						ifDeath(worldTime, roadspeed, position, score, distance, enemyspeed, timestop, minuspoints);
					}
					//pauza
					else if (event.key.keysym.sym == SDLK_p) {
						if (pause == 0)
						{
							enemyspeed;
							pause = 1;
							t3 = t2;
						}
						else {
							pause = 0;
							t2 = SDL_GetTicks();
							t4 = t2 - t3;
						}
					}
					//przyspieszenie
					else if (event.key.keysym.sym == SDLK_UP) {
						if (pause != 1) carspeed=1;
					}
					//zapis gry
					else if (event.key.keysym.sym == SDLK_s) {
						saveGame(roadspeed, score, distance, worldTime, position, enemyspeed, gamesaves, number_of_save);
					}
					//�adowanie okna z zapisami gier
					else if (event.key.keysym.sym == SDLK_l) {
						pause = 1;
						t3 = t2;
						game_saves_screen = 1;
					}
					else if (event.key.keysym.sym == SDLK_1) {
						int w=0;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_2) {
						int w = 1;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_3) {
						int w = 2;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_4) {
						int w = 3;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_5) {
						int w = 4;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_6) {
						int w = 5;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_7) {
						int w = 6;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_8) {
						int w = 7;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
					else if (event.key.keysym.sym == SDLK_9) {
						int w = 8;
						loadingButtonPressed(w, gamesaves, score, roadspeed, distance, position, worldTime, pause, enemyspeed, t2, t3, t4, game_saves_screen, number_of_save);
					}
						break;
				     //puszczenie klawisza
				case SDL_KEYUP:
					carspeed = 0;
					break;
					//wyj�cie z gry
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		};

	//zwolnienie powierzchni
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
