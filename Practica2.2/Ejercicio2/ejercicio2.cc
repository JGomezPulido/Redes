#include "../Serializable.h"

class Player : public Serializable
{
public:

    Player(const char* name, int16_t x, int16_t y);
    ~Player();

    void to_bin() override {

    };
    int from_bin(char * data) override {
        
    };
    
private:
    static const size_t MAX_NAME = 20;

    int16_t pos_x;
    int16_t pos_y;
  
    char name[MAX_NAME];
};

void main()
{

}