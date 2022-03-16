#pragma once
#include <memory>
#include <vector>

/*
    Wrapper class that represents a matrix, using smart pointers for row, and each row it's 
    a smart pointer of a vector template of type T.
*/
template <class T>
class PetriMatrix
{
private:
	std::unique_ptr<std::vector<std::unique_ptr<std::vector<T>>>> matrix;
public:
	PetriMatrix() {
        matrix = std::make_unique<std::vector<std::unique_ptr<std::vector<T>>>>();
    };

    void addRow(std::unique_ptr<std::vector<T>> row) {
        matrix->push_back(std::move(row));
    }
    const std::vector<T>& getRow(size_t row_number) {
        return *matrix->at(row_number);
    }
    void modifyElement(size_t column, size_t row, T e) {
        matrix->at(row)->at(column) = e;
    }

    const size_t rowSize() {
        return matrix->at(0)->size();
    }

    const size_t columnSize() {
        return matrix->size();
    }

    const T getElement(size_t column, size_t row) {
        return matrix->at(column)->at(row);
    }



};