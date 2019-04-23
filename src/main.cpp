/*
 * This program is designed to create a background timer that calls a function
 * provided by the user.
	Copyright(C) 2019 'illequiregimenhabet' - GitHub

	This program is free software : you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < https://www.gnu.org/licenses/>.
 */

#include "pch.h"
#include "AsyncTimer.h"
#include <iostream>

void print(int a)
{
	std::cout << a << std::endl;
}

int main() {
	AST::AsyncTimer t;
	t.SetTimeout(300);
	print(1);
	t.Run(&print, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	std::cout << "Hello World!\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));
}