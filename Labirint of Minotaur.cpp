#include <stdio.h>
#include <iostream>
#include <iterator>
#include <fstream>
#include <filesystem>
#include <queue>
#include <vector>
#include <string>


using namespace std;
using namespace filesystem;
void main();
enum Direction
{
	Up, Down, Right, Left,
	Up_Left, Up_Right, Down_Left, Down_Right
};


enum Game_State
{
	Playing, Game_Over, Win
};

int const CountStep = 8;
int const TurnXArray[] = { -1, -1, 1, 1, -1, 0 , 1, 0 };
int const TurnYArray[] = { -1, 1, -1, 1, 0, -1 , 0, 1 };
Direction const Step_Direction[] = { Up_Right, Up_Left, Down_Left, Down_Right, Right, Down, Left, Up };
vector <vector <char>> g_Field;//само игровое поле

struct Vector2 //структура которая позволяет получать координаты
{
	int X, Y;
	//float StepCost;
	Direction Local_Direction;
	Vector2()
	{

	}
	Vector2(int x, int y)
	{
		X = x;
		Y = y;
	}

	int GetDistanceTo(Vector2 Target)//получаем дистанцию по клеткам до цели
	{
		return int(abs(Target.Y - Y) + abs(Target.X - X));
	}

	void SetDirection(vector <Vector2> closeCoordinates)
	{
		//float min = FLT_MAX;
		for (int i = 0; i < CountStep; i++)
		{
			for (int j = 0; j < closeCoordinates.size(); j++)
			{
				//if (X + TurnXArray[i] == parent.X && Y + TurnYArray[i] == parent.Y)
				if (X + TurnXArray[i] == closeCoordinates[j].X && Y + TurnYArray[i] == closeCoordinates[j].Y)
				{
					Local_Direction = Step_Direction[i];
				}
			}
		}
	}
	bool isValid()
	{
		return X > -1 && Y > -1 && Y < g_Field.size() && X < g_Field[Y].size();
	}
	bool operator ==(const Vector2& other)
	{
		return X == other.X && Y == other.Y;
	}
	bool operator !=(const Vector2& other)
	{
		return X != other.X || Y != other.Y;
	}

};

void FindSmallestCost(Vector2& f_Position_Algorithm);
bool Not_In(Vector2 DesiredCoordinate, vector<Vector2> VisitedCoordinates);



vector <vector <float>> g_Enemy_Field;//поле значений у врага
vector <Vector2> g_CloseCoordinates;//посещенные клетки
vector <Vector2> OpenCoordinates;//могут быть посещены
vector <Vector2> PathToPlayer;//путь до игрока
//string path = std::boost::filesystem::current_path().string();

//путь до файла в котором описано поле
ifstream g_file_field(current_path().string() + "/Field Example.txt");
Vector2 Player_Position;//позиция игрока
Vector2 g_Enemy_Position;//текущая позиция
Vector2 g_Win_Position;
Vector2 g_Next_Position_Algorithm;//следующая позиция алгоритма



void UpdateField()//выводим обновленную карту
{
	system("cls");
	for (int i = 0; i < g_Field.size(); i++)
	{
		for (int j = 0; j < g_Field[i].size(); j++)
		{
			cout << g_Field[i][j];
		}
		cout << endl;
	}
	cout << endl;
}

void UpdateEnemyField()//обновляем все данные врага, чтобы он мог найти путь
{
	for (int i = 0; i < g_Field.size(); i++)
	{
		for (int j = 0; j < g_Field[i].size(); j++)
		{
			g_Enemy_Field[i][j] = -1;
		}
	}
	PathToPlayer.clear();
	g_CloseCoordinates.clear();
	OpenCoordinates.clear();
}

void MakePathToPlayer(Vector2 f_Position_Algorithm)//собираем в отдельный список дорогу до игрока
{
	Vector2 next_Position_From_Player = f_Position_Algorithm;

	PathToPlayer.push_back(f_Position_Algorithm);

	while (g_CloseCoordinates.empty() == false)
	{
		for (int i = 0; i < CountStep; i++)
		{
			//если направление совпадает с одним из значений, то расчитываем следующую клетку
			if (f_Position_Algorithm.Local_Direction == Step_Direction[i])
			{
				next_Position_From_Player.X = f_Position_Algorithm.X - TurnXArray[i];
				next_Position_From_Player.Y = f_Position_Algorithm.Y - TurnYArray[i];
				//if()
				for (vector <Vector2>::iterator it = g_CloseCoordinates.begin(); it != g_CloseCoordinates.end(); it++)
				{
					if (next_Position_From_Player == g_Enemy_Position)
					{
						g_CloseCoordinates.erase(it);
						break;
					}
					else
					{
						if (next_Position_From_Player == *it)
						{
							next_Position_From_Player.Local_Direction = it->Local_Direction;
							g_CloseCoordinates.erase(it);
							PathToPlayer.push_back(next_Position_From_Player);
							break;
						}
					}
				}

				break;
			}
		}
		f_Position_Algorithm = PathToPlayer.back();
	}
}

void FindPathToPlayer(Vector2 EnemyPosition)//ищем путь к игроку
{

	Vector2 NextPosition, Position;//следующая позиция и позиция от которой берется новая
	//EnemyPosition.StepCost = 0;
	g_CloseCoordinates.push_back(EnemyPosition);

	g_Enemy_Field[EnemyPosition.Y][EnemyPosition.X] = 0;

	while (Player_Position != g_Next_Position_Algorithm)
	{
		Position = g_CloseCoordinates.back();
		for (int i = 0; i < CountStep; i++)
		{
			NextPosition = Vector2(Position.X + TurnXArray[i], Position.Y + TurnYArray[i]);
			//NextPosition.StepCost = 
			if (NextPosition.X > -1 && NextPosition.Y > -1 && NextPosition.Y < g_Field.size() && NextPosition.X < g_Field[NextPosition.Y].size())
			{
				if (g_Field[NextPosition.Y][NextPosition.X] != 'B' &&
					Not_In(NextPosition, g_CloseCoordinates)/* && Not_In(NextPosition, OpenCoordinates)*/)//условие чтобы не добавлять в открытую очередь вершины которые там уже имеются
				{
					//if(g_Enemy_Field[NextPosition.Y][NextPosition.X] > g_Enemy_Field[Position.Y][Position.X])
					if (g_Enemy_Field[NextPosition.Y][NextPosition.X] > g_Enemy_Field[Position.Y][Position.X]
						+ sqrt((float)abs(TurnXArray[i] + TurnYArray[i]))
						|| g_Enemy_Field[NextPosition.Y][NextPosition.X] == -1)//проверяем можем ли мы добраться в уже существующую точку быстрее если да, то меняем значение
					{
						g_Enemy_Field[NextPosition.Y][NextPosition.X] = g_Enemy_Field[Position.Y][Position.X]
							+ sqrt((float)abs(TurnXArray[i] + TurnYArray[i]));

						NextPosition.Local_Direction = Step_Direction[i];
						OpenCoordinates.push_back(NextPosition);
					}
				}
			}
		}
		FindSmallestCost(g_Next_Position_Algorithm);//чтобы найти самую дешевую клетку
		if (OpenCoordinates.empty())//если нет вариантов, то заканчиваем рассчитывать путь
		{
			break;
		}
	}

	MakePathToPlayer(g_Next_Position_Algorithm);//после нахождения всех возможных ходов строим путь до игрока
}

void FindSmallestCost(Vector2& f_Position_Algorithm)//находим самую маленькую по стоимости шага клетку
{
	float smallestDistance = FLT_MAX;
	vector <Vector2> ::iterator it = OpenCoordinates.begin();
	vector <Vector2> ::iterator smallestIterator;
	Vector2 old_Enemy_Pos;
	if (g_CloseCoordinates.size() > 1)
	{
		old_Enemy_Pos.X = f_Position_Algorithm.X;
		old_Enemy_Pos.Y = f_Position_Algorithm.Y;
	}
	else
	{
		old_Enemy_Pos.X = g_Enemy_Position.X;
		old_Enemy_Pos.Y = g_Enemy_Position.Y;
	}
	//int iterator;
	for (; it != OpenCoordinates.end(); it++)
	{
		if (g_Enemy_Field[it->Y][it->X] + (float)it->GetDistanceTo(Player_Position) <= smallestDistance && Not_In(*it, g_CloseCoordinates))
		{
			if (it->X == 11 && it->Y == 6)
			{
				cout << smallestDistance << endl;
				//cout << g_Enemy_Field[NextPosition.Y][NextPosition.X] << " " << sqrt((float)abs(TurnXArray[i] + TurnYArray[i])) << endl;
			}
			smallestDistance = g_Enemy_Field[it->Y][it->X] + (float)it->GetDistanceTo(Player_Position);
			if (it->X == 11 && it->Y == 6)
			{
				cout << smallestDistance << endl;
				//cout << g_Enemy_Field[NextPosition.Y][NextPosition.X] << " " << sqrt((float)abs(TurnXArray[i] + TurnYArray[i])) << endl;
			}
			f_Position_Algorithm = *it;
			smallestIterator = it;
		}
	}

	//f_Position_Algorithm.SetDirection(g_CloseCoordinates);//находим направление от новой точки до самой ближайшей нужной
	OpenCoordinates.erase(smallestIterator);// удаляем чтобы не пробегать по ней снова
	g_CloseCoordinates.push_back(f_Position_Algorithm);
}

bool Not_In(Vector2 DesiredCoordinate, vector<Vector2> VisitedCoordinates)//проверка есть ли в приведенном списке координат определенная координата
{
	for (int i = 0; i < VisitedCoordinates.size(); i++)
	{
		if (DesiredCoordinate == VisitedCoordinates[i])
		{
			return false;
		}
	}
	return true;
}

void GetField()//считывание поля с файла
{
	int count_lines = 0;
	string LineField;

	while (getline(g_file_field, LineField))
	{
		int Player_Pos_X = 0, Enemy_Pos_X = 0, Win_Pos_X = 0;
		count_lines++;
		g_Field.resize(count_lines);
		g_Enemy_Field.resize(count_lines);
		for (char cell : LineField)
		{
			if (cell == 'P')//считываем клетку игрока
			{
				Player_Position.X = Player_Pos_X;
				Player_Position.Y = g_Enemy_Field.size() - 1;
			}
			else
			{
				Player_Pos_X++;
			}

			if (cell == 'E')//считываем позицию клетки врага
			{
				g_Enemy_Position.X = Enemy_Pos_X;
				g_Enemy_Position.Y = g_Enemy_Field.size() - 1;
			}
			else
			{
				Enemy_Pos_X++;
			}

			if (cell == 'W')
			{
				g_Win_Position.X = Win_Pos_X;
				g_Win_Position.Y = g_Enemy_Field.size() - 1;
			}
			else
			{
				Win_Pos_X++;
			}
			g_Enemy_Field[g_Enemy_Field.size() - 1].push_back(-1);
			g_Field[g_Field.size() - 1].push_back(cell);
		}
	}
}



void Switch_Player_Position_Check_Enemy(Vector2 f_Next_Position, Game_State& f_state)
{
	bool returnValue;
	if (g_Field[f_Next_Position.Y][f_Next_Position.X] != 'B')
	{

		swap(g_Field[f_Next_Position.Y][f_Next_Position.X], g_Field[Player_Position.Y][Player_Position.X]);
		//g_Field[Player_Position.Y][Player_Position.X] = 'O';
		Player_Position = f_Next_Position;


		//g_Field[Player_Position.Y][Player_Position.X] = 'P';
	}
	if (f_Next_Position == g_Win_Position)
		f_state = Win;
	else
		f_Next_Position == g_Enemy_Position ? f_state = Game_Over : f_state = Playing;
}

Game_State MakeTurn(char StepCommand)//человек делает ход
{
	Vector2 Next_Position;
	Game_State return_Game_State = Playing;

	switch (StepCommand)//можно сделать пробежку по массиву, но т.к. команд мало, то switch подойдет
	{
	case 'U':
		//cout << "UP" << endl;
		Next_Position.Y = Player_Position.Y - 1;
		Next_Position.X = Player_Position.X;
		if (Next_Position.Y >= 0)
		{
			Switch_Player_Position_Check_Enemy(Next_Position, return_Game_State);

		}
		break;
	case'R':
		Next_Position.Y = Player_Position.Y;
		Next_Position.X = Player_Position.X + 1;
		if (Next_Position.X < g_Field[Next_Position.Y].size())
		{
			Switch_Player_Position_Check_Enemy(Next_Position, return_Game_State);
		}
		break;
	case'L':
		Next_Position.Y = Player_Position.Y;
		Next_Position.X = Player_Position.X - 1;
		if (Next_Position.X >= 0)
		{
			Switch_Player_Position_Check_Enemy(Next_Position, return_Game_State);
		}
		break;
	case'D':
		Next_Position.Y = Player_Position.Y + 1;
		Next_Position.X = Player_Position.X;
		if (Next_Position.Y < g_Field.size())
		{
			Switch_Player_Position_Check_Enemy(Next_Position, return_Game_State);
		}
		break;
	default:
		//printf("I don't know this command");
		break;
	}


	UpdateField();

	return return_Game_State;


}


void MakeEnemyStep(Game_State& f_game_state)//Враг ходит до игрока сразу
{
	if (PathToPlayer.back() != Player_Position)
	{
		if (!PathToPlayer.empty())
		{
			swap(g_Field[g_Enemy_Position.Y][g_Enemy_Position.X], g_Field[PathToPlayer.back().Y][PathToPlayer.back().X]);
			g_Enemy_Position = PathToPlayer.back();
		}
		UpdateField();
	}
	else
	{
		f_game_state = Game_Over;
	}
}

void EnemyTurn(Game_State& f_game_state)//очередь вражины ходить
{
	float minimalDistance = FLT_MAX;
	FindPathToPlayer(g_Enemy_Position);

	MakeEnemyStep(f_game_state);
	UpdateEnemyField();
}


void main()
{
	Game_State current_game_state;
	if (g_file_field.is_open())
	{
		GetField();
	}
	else
	{
		printf("No such file in directory. I don't know why but you must write path by yourself");
		system("pause");
	}
	UpdateField();
	while (true)
	{

		char Player_turn;
		cin >> Player_turn;
		current_game_state = MakeTurn(Player_turn);
		if (current_game_state != Playing)
		{
			break;
		}
		else
		{
			EnemyTurn(current_game_state);
		}
	}
	current_game_state == Win ?
		printf("GG") : printf("LOOOOOOOOSER");

	system("pause");
}
