//
// Created by vhundef on 25.12.2019.
//

#ifndef PROGONHLANG_GAME_FIELD_HPP
#define PROGONHLANG_GAME_FIELD_HPP

#include <SDL_log.h>
#include <SDL_rect.h>
#include <time.h>

#include <ctime>
#include <fstream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "UI/ui_manager.hpp"
#include "cell.hpp"

using namespace std;

class GameField {
 private:
  ScreenManager *screenManager;
  int aliveCells = 0;

 public:
  [[nodiscard]] int getAliveCells() const {
	return aliveCells;
  }

  std::vector<std::vector<Cell>> cells;///< Хранит данные о клетках которые находятся на экране

  explicit GameField(ScreenManager *_screenManager) {
	if (_screenManager == nullptr) {
	  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error: GameField->constructor: _screenManager is null");
	  throw std::runtime_error("Error: GameField->constructor: _screenManager is null, this shouldn't have happened");
	}
	screenManager = _screenManager;
	int h = screenManager->getWindowResolutionY(), w = screenManager->getWindowResolutionX();

	for (int i = 0; i <= h / 16; ++i) {///< Определяет сколько будет клеток на оси Y
	  vector<Cell> row;
	  for (int j = 0; j <= w / 16; j++) {///< Определяет сколько будет клеток на оси X
		row.emplace_back();
		row.back().setLocation(j, i, screenManager->getWindow());///< Задаёт клетке местоположение
	  }
	  cells.push_back(row);
	}
	randomiseField();///< Заполняет поле псевдо-случайным способом
  }
  ///@brief Заполняет поле живыми клетками случайным способом
  void randomiseField() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 10000);
	for (auto &row : cells) {
	  for (auto &cell : row) {
		unsigned int num = dis(gen);
		if (num % 5 != 0) {
		  cell.setState('u');
		} else {
		  cell.setState('a');
		}
	  }
	}
  }

  ///@brief отчищает поле от клеток
  void clearBoard() {
	for (auto &cell : cells) {
	  for (int j = 0; j < cells[0].size(); j++) {
		cell[j].setState('d');
	  }
	}
  }

  ///@brief перерисовывает/рисует поле
  void drawBoard() {
	if (cells.empty()) {
	  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "GameField->drawBoard: Cells array is empty");
	  //throw std::runtime_error("GameField->drawBoard: Cells array is NULL");
	}

	for (auto &row : cells) {
	  for (auto &cell : row) {
		cell.redraw(screenManager->getRenderer());
	  }
	}
  }

  /// @brief Проверяет каждую клетку на соблюдение правил
  void checkRulesCompliance() {
	int neighbors = 0;
	aliveCells = countAliveCells();
	for (int i = 0; i < cells.size(); ++i) {
	  for (int j = 0; j < cells[0].size(); ++j) {
		if (getElement(i, j - 1)->getState() == 'a') neighbors++;    //checking cell on the left
		if (getElement(i, j + 1)->getState() == 'a') neighbors++;    //checking cell on the right
		if (getElement(i - 1, j)->getState() == 'a') neighbors++;    //checking cell on the top
		if (getElement(i + 1, j)->getState() == 'a') neighbors++;    //checking cell on the bottom
		if (getElement(i - 1, j - 1)->getState() == 'a') neighbors++;//checking cell on the top left
		if (getElement(i - 1, j + 1)->getState() == 'a') neighbors++;//checking cell on the top right
		if (getElement(i + 1, j - 1)->getState() == 'a') neighbors++;//checking cell on the bottom left
		if (getElement(i + 1, j + 1)->getState() == 'a') neighbors++;//checking cell on the bottom right
		if (neighbors < 2) {                                         ///< Если у клетки мало соседей то она умирает
		  cells[i][j].next_state = 'd';
		} else if (neighbors > 3) {///< Если у клетки больше 3 соседей, то она умирает
		  cells[i][j].next_state = 'd';
		} else if (neighbors == 3) {///< Если у (пустой/мёртвой)клетки 3-е соседей, то создаётся новая клетка
		  cells[i][j].next_state = 'b';
		}
		neighbors = 0;
	  }
	}
	for (auto &cell : cells) {///< Применяем ко всем клеткам новое состояние
	  for (int j = 0; j < cells[0].size(); ++j) {
		cell[j].applyNewState();
	  }
	}
  }
  ///@brief Считет количество живых клеток
  int countAliveCells() {
	int _aliveCells = 0;
	for (int i = 0; i < cells.size(); ++i) {
	  for (int j = 0; j < cells[0].size(); ++j) {
		if (getElement(i, j)->getState() == 'a') _aliveCells++;
	  }
	}
	return _aliveCells;
  }

  ///@returns Возвращает ссылку на объект из матрицы клеток
  Cell *getElement(int column, int row) {
	if (column < 0)
	  column = cells.size() - 1;
	if (column == cells.size())
	  column = 0;
	if (row < 0)
	  row = cells[column].size() - 1;
	if (row == cells[column].size())
	  row = 0;
	return &cells[column][row];
  }

  ///@returns Возвращает ссылку на объект из матрицы клеток
  Cell *getElement(twoInt coords) {
	if (coords.a < 0)
	  coords.a = cells.size() - 1;
	if (coords.a == cells.size())
	  coords.a = 0;
	if (coords.b < 0)
	  coords.b = cells[coords.a].size() - 1;
	if (coords.b == cells[coords.a].size())
	  coords.b = 0;
	return &cells[coords.a][coords.b];
  }
  ///@returns Возвращает матрицу содержащую все клетки поля
  std::vector<std::vector<Cell>> getField() {
	return cells;
  }

  /**@brief позволяет сменить поле
   * @note Используется при откате поля к версии которая была до редактирования
   * **/
  void setField(std::vector<std::vector<Cell>> field) {
	cells = std::move(field);
  }
};

#endif//PROGONHLANG_GAME_FIELD_HPP
