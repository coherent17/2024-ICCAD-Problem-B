#include "Util.h"

Param::Param():
    MAX_SQUARE_DISPLACEMENT(400000000),
    MAX_BANDWIDTH(10000)
    {}
Param::~Param(){}

double SquareEuclideanDistance(const Coor &p1, const Coor &p2){
    return std::pow(p1.getX() - p2.getX(), 2) + std::pow(p1.getY() - p2.getY(), 2);
}


double MangattanDistance(const Coor &p1, const Coor &p2){
    return std::abs(p1.getX() - p2.getX()) + std::abs(p1.getY() - p2.getY());
}

double GaussianKernel(const Coor &p1, const Coor &p2, double bandwidth){
    double numerator = SquareEuclideanDistance(p1, p2);
    double denominator = -2 * std::pow(bandwidth, 2);
    return std::exp(numerator / denominator);

}

double HPWL(const Coor& c1, const Coor& c2){
    return std::abs(c1.x - c2.x) + std::abs(c1.y - c2.y);
}

bool IsOverlap(const Coor &coor1, double w1, double h1, const Coor &coor2, double w2, double h2){
    // Check if one rectangle is to the left of the other
    if(coor1.x + w1 <= coor2.x || coor2.x + w2 <= coor1.x){
        return false;
    }

    // Check if one rectangle is above the other
    if (coor1.y + h1 <= coor2.y || coor2.y + h2 <= coor1.y) {
            return false;
    }
    return true;
}

std::string toStringWithPrecision(double value, int numAfterDot){
    return std::to_string(value).substr(0, std::to_string(value).find('.') + numAfterDot);
}

void update_bar(int percent_done){
	int num_char = percent_done * PRO_BAR_LENGTH / 100;
	printf("\r[");
	for(int i = 0; i < num_char; i++){
		printf("#");
	}
	for(int i = 0; i < PRO_BAR_LENGTH - num_char; i++){
		printf(" ");
	}
	printf("] %d%% Done", percent_done);
	fflush(stdout);
}