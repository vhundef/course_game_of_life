//
// Created by vhundef on 24.12.2019.
//

#ifndef PROGONHLANG_SCREENMANAGER_HPP
#define PROGONHLANG_SCREENMANAGER_HPP

#include <SDL_ttf.h>

#include <iostream>

#include "../input_manager.hpp"

class ScreenManager {
 private:
  TTF_Font *font{};
  SDL_Renderer *renderer;
  SDL_DisplayMode DM{};
  int fontSize = {};
  int windowResolutionX{0};
  int windowResolutionY{0};
  InputManager *inputManager;
  SDL_Window *window{};
  struct twoInt {
	int a;
	int b;
  };

 public:
  std::string fontName = "Roboto-Medium";

  ///@brief Возвращает ссылку на окно
  [[nodiscard]] SDL_Window *getWindow() const {
	return window;
  }
  ///@brief возвращат разрешение экрана по оси X
  [[nodiscard]] int getWindowResolutionX() const {
	return windowResolutionX;
  }
  ///@brief возвращат разрешение экрана по оси X
  [[nodiscard]] int getWindowResolutionY() const {
	return windowResolutionY;
  }
  ///@brief возвращает ссылку на рендерер
  [[nodiscard]] SDL_Renderer *getRenderer() const {
	return renderer;
  }
  ///@brief возвращает размер шрифта
  [[nodiscard]] int getFontSize() const {
	return fontSize;
  }
  ///@brief возвращает ссылку на менеджер событий
  [[nodiscard]] InputManager *getInputManager() const {
	return inputManager;
  }

 public:
  explicit ScreenManager(InputManager *pInputManager) {
	SDL_GetCurrentDisplayMode(0, &DM);
	auto Width = DM.w;
	auto Height = DM.h;
	window = SDL_CreateWindow("The Game Of Life", 0, 0, Width, Height, SDL_WINDOW_SHOWN);
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	if (window == nullptr) {
	  std::string error = SDL_GetError();
	  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", error.c_str());
	  throw std::runtime_error("Unable to create window (SDL2)");
	}
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == nullptr) {
	  SDL_DestroyWindow(window);
	  std::string error = SDL_GetError();
	  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", error.c_str());
	  throw std::runtime_error("Unable to create render (SDL2)");
	}

	if (TTF_Init() == -1) {
	  std::string error = TTF_GetError();
	  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", error.c_str());
	  throw std::runtime_error("ScreenManager->TTF_OpenFont: Attempt to open font was unsuccessful");
	}

	font = TTF_OpenFont((fontName + ".ttf").c_str(), 16);
	if (!font) {
	  std::string error = TTF_GetError();
	  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", error.c_str());
	  throw std::runtime_error("ScreenManager->TTF_OpenFont: Attempt to open font was unsuccessful");
	}
	inputManager = pInputManager;
	SDL_GetWindowSize(window, &windowResolutionX, &windowResolutionY);
  }

  /**
   * @brief позволяет изменить размер шрифта
   * @param size размер шрифта
  **/
  void changeFontSize(int size) {
	TTF_CloseFont(font);
	font = TTF_OpenFont((fontName + ".ttf").c_str(), size);
	if (!font) {
	  std::string error = TTF_GetError();
	  SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", error.c_str());
	  throw std::runtime_error("ScreenManager->changeFontSize: Attempt to change font size was unsuccessful");
	}
	fontSize = size;
  }

  /**@brief печатает текст
   * @param text текст который нужно напечатать
   * @param x местоположение по оси X
   * @param y Местоположение по оси Y
   * @param color Цвет текста
   * @param font_size размер текста
   * @param centred Должен ли тескст быть по центру указанных координат
   */
  void printText(const std::string &text, int x, int y, SDL_Color color = {0, 0, 0}, int font_size = 16, bool centred = false) {
	if (getFontSize() != font_size)
	  changeFontSize(font_size);
	auto surfaceMessage = TTF_RenderUTF8_Solid(font, text.c_str(), color);// as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
	auto Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);//now you can convert it into a texture
	SDL_Rect Message_rect;                                                //create a rect
    if (centred) {
      Message_rect.x = x - getTextSize(text, font_size).a / 2;  //controls the rect's x coordinate
      Message_rect.y = y - getTextSize(text, font_size).b / 2; // controls the rect's y coordinate
    } else {
      Message_rect.x = x;  //controls the rect's x coordinate
      Message_rect.y = y; // controls the rect's y coordinate
    }
	TTF_SizeUTF8(font, text.c_str(), &Message_rect.w, &Message_rect.h);
	SDL_RenderCopy(renderer, Message, nullptr, &Message_rect);//you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture
	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(Message);
	//  delete Message;
  }
  ///@brief Используется для перевода цвета из формата SDL_Color в формат Hex
  static int rgbToHex(SDL_Color rgb) {///< @brief used to convert SDL_Color to hex
	return rgb.r * rgb.g * rgb.b;
  }
  /**@brief Возращает размер текста
   * @param _string текст размер котрого нужно узнать
   * @param _fontSize размер шрифта
   * @return размер текста в формате {x,y}
   */
  twoInt getTextSize(const std::string &_string, int _fontSize) {
	changeFontSize(_fontSize);
	twoInt widthAndHeight{};
	TTF_SizeUTF8(font, _string.c_str(), &widthAndHeight.a, &widthAndHeight.b);
	return widthAndHeight;
  }
};

#endif//PROGONHLANG_SCREENMANAGER_HPP