#include <stdio.h>
#include <iostream>
#include <iterator>
#include <fstream>

#include <queue>
#include <vector>
#include <string>


using namespace std;

enum Direction
{
	Up, Down, Right, Left,
	Up_Left, Up_Right, Down_Left, Down_Right
};

int const CountStep = 8;
int const TurnXArray[] = { -1, -1, 1, 1, 1, 0 , -1, 0};
int const TurnYArray[] = { -1, 1, -1, 1, 0, 1 , 0, -1};
Direction const Step_Direction[] = { Up_Left, Down_Left, Up_Right, Down_Right, Right, Up, Left, Down};


struct Vector2 //структура которая позволяет получать координаты
{
	int X, Y;
	Direction Local_Direction;
	Vector2()
	{

	}
	Vector2(int x, int y)
	{
		X = x;
		Y = y;
	}

	float GetDistanceTo(Vector2 Target)//получаем дистанцию по клеткам до цели
	{
		return float(abs(Target.Y - Y) + abs(Target.X - X));
	}
	//идет не правильное получение направления движения (я хз как сделать)
	void SetDirection(vector <Vector2> closeCoordinates)
	{
		//float min = FLT_MAX;
		for (int i = 0; i < CountStep; i++)
		{
			for (int j = 0; j < closeCoordinates.size(); j++)
			{
				//if (X + TurnXArray[i] == parent.X && Y + TurnYArray[i] == parent.Y)
				if(X + TurnXArray[i] == closeCoordinates[j].X && Y + TurnYArray[i] == closeCoordinates[j].Y)
				{
					Local_Direction = Step_Direction[i];
				}
			}
		}
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

void FindSmallestCost(Vector2 &f_Position_Algorithm);
bool Not_In(Vector2 DesiredCoordinate, vector<Vector2> VisitedCoordinates);


vector <vector <char>> g_Field;//само игровое поле
vector <vector <float>> g_Enemy_Field;//поле значений у врага
vector <Vector2> g_CloseCoordinates;//посещенные клетки
vector <Vector2> OpenCoordinates;//могут быть посещены
vector <Vector2> PathToPlayer;//путь до игрока


ifstream g_file_field("E:\\Field Example.txt");//путь до файла в котором описано поле

Vector2 Player_Position;//позиция игрока
Vector2 g_Enemy_Position;//текущая позиция
Vector2 g_Next_Position_Algorithm;//следующая позиция алгоритма



void UpdateField()//выводим обновленную карту
{
	//system("cls");
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

void MakePathToPlayer(Vector2 &f_Position_Algorithm)//собираем в отдельный список дорогу до игрока
{
	Vector2 next_Position_From_Player = f_Position_Algorithm;

	PathToPlayer.push_back(f_Position_Algorithm);

	while (next_Position_From_Player != g_Enemy_Position)
	{
		for (int i = 0; i < CountStep; i++)
		{
			//если направление совпадает с одним из значений, то расчитываем следующую клетку
			if (f_Position_Algorithm.Local_Direction == Step_Direction[i])
			{
				next_Position_From_Player.X = f_Position_Algorithm.X + TurnXArray[i];
				next_Position_From_Player.Y = f_Position_Algorithm.Y + TurnYArray[i];
				for (vector <Vector2>::iterator it = g_CloseCoordinates.begin(); it != g_CloseCoordinates.end(); it++)
				{
					if (next_Position_From_Player == *it)
					{
						next_Position_From_Player.Local_Direction = it->Local_Direction;
						g_CloseCoordinates.erase(it);
						break;
					}
				}
				PathToPlayer.push_back(next_Position_From_Player);
				break;
			}
		}
		f_Position_Algorithm = next_Position_From_Player;
	}
}

void FindPathToPlayer(Vector2 EnemyPosition)//ищем путь к игроку
{

	Vector2 NextPosition, Position;//следующая позиция и позиция от которой берется новая

	g_CloseCoordinates.push_back(EnemyPosition);

	g_Enemy_Field[EnemyPosition.Y][EnemyPosition.X] = 0;

	do
	{
		Position = g_CloseCoordinates.back();
		for (int i = 0; i < CountStep; i++)
		{
			NextPosition = Vector2(Position.X + TurnXArray[i], Position.Y + TurnYArray[i]);
			if (NextPosition.X > -1 && NextPosition.Y > -1 && NextPosition.Y < g_Field.size() && NextPosition.X < g_Field[NextPosition.Y].size())
			{
				if (g_Field[NextPosition.Y][NextPosition.X] != 'B' &&
					Not_In(NextPosition, g_CloseCoordinates) && Not_In(NextPosition, OpenCoordinates))//условие чтобы не добавлять в открытую очередь вершины которые там уже имеются
				{
					OpenCoordinates.push_back(NextPosition);

					if (g_Enemy_Field[NextPosition.Y][NextPosition.X] > g_Enemy_Field[Position.Y][Position.X]
						+ sqrt((float)abs(TurnXArray[i] + TurnYArray[i]))
						 || g_Enemy_Field[NextPosition.Y][NextPosition.X] == -1)//проверяем можем ли мы добраться в уже существующую точку быстрее если да, то меняем значение
						g_Enemy_Field[NextPosition.Y][NextPosition.X] = g_Enemy_Field[Position.Y][Position.X]
						+ sqrt((float)abs(TurnXArray[i] + TurnYArray[i]));
				}
			}
		}
		FindSmallestCost(g_Next_Position_Algorithm);//чтобы найти самую дешевую клетку
		if (OpenCoordinates.empty())//если нет вариантов, то заканчиваем рассчитывать путь
		{
			break;
		}
	} while (Player_Position != g_Next_Position_Algorithm);

	MakePathToPlayer(g_Next_Position_Algorithm);//после нахождения всех возможных ходов строим путь до игрока
}

void FindSmallestCost(Vector2 &f_Position_Algorithm)//находим самую маленькую по стоимости шага клетку
{
	float smallestDistance = 1000000000.f;
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
		if (g_Enemy_Field[it->Y][it->X] + it->GetDistanceTo(Player_Position) < smallestDistance && Not_In(*it, g_CloseCoordinates))
		{
			smallestDistance = g_Enemy_Field[it->Y][it->X] + it->GetDistanceTo(Player_Position);
			f_Position_Algorithm = *it;
			//smallestIterator = it;
		}
	}
	f_Position_Algorithm.SetDirection(g_CloseCoordinates);//находим направление от новой точки до самой ближайшей нужной
	//OpenCoordinates.erase(smallestIterator);// удаляем чтобы не пробегать по ней снова
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
		int Player_Pos_X = 0, Enemy_Pos_X = 0;
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

			g_Enemy_Field[g_Enemy_Field.size() - 1].push_back(-1);
			g_Field[g_Field.size() - 1].push_back(cell);
		}
	}
}

bool MakeTurn(char StepCommand)//человек делает ход
{
	Vector2 Next_Position;
	bool retunValue = false;

	switch (StepCommand)//можно сделать пробежку по массиву, но т.к. команд мало, то switch подойдет
	{
	case 'U':
		//cout << "UP" << endl;
		Next_Position.Y = Player_Position.Y - 1;
		Next_Position.X = Player_Position.X;
		if(Next_Position.Y >= 0)
		if (g_Field[Next_Position.Y][Next_Position.X] != 'B')
		{
			retunValue = g_Field[Next_Position.Y][Next_Position.X] == 'W';
			swap(g_Field[Next_Position.Y][Next_Position.X], g_Field[Player_Position.Y][Player_Position.X]);
			//g_Field[Player_Position.Y][Player_Position.X] = 'O';
			Player_Position = Next_Position;
			//g_Field[Player_Position.Y][Player_Position.X] = 'P';
		}
		break;
	case'R':
		Next_Position.Y = Player_Position.Y;
		Next_Position.X = Player_Position.X + 1;
		if (Next_Position.X < g_Field[Next_Position.Y].size())
			if (g_Field[Next_Position.Y][Next_Position.X] != 'B')
			{
				retunValue = g_Field[Next_Position.Y][Next_Position.X] == 'W';
				swap(g_Field[Next_Position.Y][Next_Position.X], g_Field[Player_Position.Y][Player_Position.X]);
				//g_Field[Player_Position.Y][Player_Position.X] = 'O';
				Player_Position = Next_Position;
				//g_Field[Player_Position.Y][Player_Position.X] = 'P';
			}
		break;
	case'L':
		Next_Position.Y = Player_Position.Y;
		Next_Position.X = Player_Position.X - 1;
		if (Next_Position.X >= 0)
			if (g_Field[Next_Position.Y][Next_Position.X] != 'B')
			{
				retunValue = g_Field[Next_Position.Y][Next_Position.X] == 'W';
				swap(g_Field[Next_Position.Y][Next_Position.X], g_Field[Player_Position.Y][Player_Position.X]);
				Player_Position = Next_Position;
			}
		break;
	case'D':
		Next_Position.Y = Player_Position.Y + 1;
		Next_Position.X = Player_Position.X;
		if (Next_Position.Y < g_Field.size())
			if (g_Field[Next_Position.Y][Next_Position.X] != 'B')
			{
				retunValue = g_Field[Next_Position.Y][Next_Position.X] == 'W';
				swap(g_Field[Next_Position.Y][Next_Position.X], g_Field[Player_Position.Y][Player_Position.X]);
				Player_Position = Next_Position;
			}
		break;
	default:
		//printf("I don't know this command");
		break;
	}

	 
	UpdateField();

	return retunValue;
	
	
}

void MakeEnemyStep()//Враг ходит до игрока сразу
{
	for (int i = PathToPlayer.size() - 1; i >= 0; i--)
	{
		swap(g_Field[g_Enemy_Position.Y][g_Enemy_Position.X], g_Field[PathToPlayer[i].Y][PathToPlayer[i].X]);
		g_Enemy_Position = PathToPlayer[i];
		UpdateField();
	}
}

void EnemyTurn()//очередь врага ходить
{
	float minimalDistance = 1000000000.f;
	FindPathToPlayer(g_Enemy_Position);

	MakeEnemyStep();
	UpdateEnemyField();
}



int main()
{
	if(g_file_field.is_open())
	{
		GetField();
	}
	else
	{
		printf("No such file in directory. I don't know why but you must write path by yourself");
		system("pause");
		return 0;
	}
	UpdateField();
	while (true)
	{
		
		char Player_turn;
		cin >> Player_turn;
		if (MakeTurn(Player_turn))
		{
			break;
		}
		else
		{
			EnemyTurn();
		}
	}
	printf("GG");

	system("pause");
	return 0;
}


