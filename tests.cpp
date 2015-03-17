#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>

#include "limvec.h"

using namespace std;

class tester {
    const tester* self;
    static int limit;
public:
    tester() :self(this) {
        if(livecount >= limit) throw std::out_of_range("Nauseous constructor");
        livecount++;
    }
    tester(const tester& other) :self(this) { 
        if(livecount >= limit) throw std::out_of_range("Nauseous constructor");
        livecount++; 
    }
    ~tester() {
        assert(self==this);
        // This check ensures that destructor is not called on 
        // an uninitialized object
        --livecount;
    }
    tester& operator=(const tester& b) {
        assert(self==this && b.self == &b);
        return *this;
    }
    bool operator==(const tester& other) {
        return true;
    }
    bool operator!=(const tester& other) {
        return false;
    }
    static void set_limit(unsigned int i) {
        limit = i;
    }
    static int livecount;
};

int tester::livecount = 0;
int tester::limit = 1;


// Note: all tester constructor exceptions are set 
// such that they throw midway during a batch of 
// constructions to test whether the vector
// destroys the successfully constructed objects
void test()
{
    tester a;
    assert(tester::livecount == 1);


    ///////////////////////////////////////
    // Testing initilization constructor //
    ///////////////////////////////////////
    tester::set_limit(3);
    try {    
        limited_vector<tester> vec(4, a);
    }   
    catch(std::out_of_range e) {
        assert(tester::livecount == 1);
    }

    tester::set_limit(5); // 4 + 1
    limited_vector<tester> vec(4, a);

    assert(tester::livecount == 5);
    assert(vec.size() == 4);
    assert(vec.capacity() == 4);

    //////////////////////////////
    // Testing copy constructor //
    //////////////////////////////
    tester::set_limit(7);
    try {
        auto vec_b = vec;
    }
    catch(std::out_of_range e) {
        assert(tester::livecount == 5);
    }

    tester::set_limit(9); // 4 * 2 + 1
    auto vec_b = vec;
    assert(tester::livecount == 9);
    assert(vec == vec_b);
    assert(vec_b.size() == vec.size());
    assert(vec_b.capacity() == vec.capacity());

    ///////////////////////
    // Testing reserve() //
    ///////////////////////
    tester::set_limit(11);
    tester* initial_begin = vec.begin();
    tester* initial_end = vec.end();
    try {
        vec.reserve(5); 
        // will fail because it copy constructs new contents before destroying old ones
    }
    catch(std::out_of_range e) {
        assert(tester::livecount == 9);
        assert(vec == vec_b); //strong guarantee
        assert(vec.begin() == initial_begin); //references not invalidated
        assert(vec.end() == initial_end);
    }


    tester::set_limit(13); // 4 * 3 + 1
    vec.reserve(5);
    assert(tester::livecount == 9);
    assert(vec.size() == 4);
    assert(vec.capacity() == 5);
    assert(vec == vec_b); // logically equivalent

    /////////////////////////
    // Testing push_back() //
    /////////////////////////
    vec_b = vec;
    initial_begin = vec.begin();
    initial_end = vec.end();
    assert(tester::livecount == 9);
    tester::set_limit(9); // 4 * 2 + 1
    try {
        vec.push_back(a);
    }
    catch(std::out_of_range e) { //throws because of constructor
        assert(tester::livecount == 9);
        assert(vec == vec_b); //strong guarantee
        assert(vec.begin() == initial_begin); //references not invalidated
        assert(vec.end() == initial_end);
    }

    tester::set_limit(10); // 4 * 2 + 1 + 1
    vec.push_back(a); // successful push_back
    assert(tester::livecount == 10);
    assert(vec.size() == 5);

    tester::set_limit(15); // 5 * 2 + 4 + 1
    vec_b = vec;
    initial_begin = vec.begin();
    initial_end = vec.end();
    assert(tester::livecount == 11);
    tester::set_limit(12); //5 * 2 + 1 + 1
    try {
        vec.push_back(a);
    }
    catch(std::overflow_error e) // throws because of vector capacity
    {
        assert(tester::livecount == 11);
        assert(vec == vec_b); //logically equivalent
        assert(vec.begin() == initial_begin); //references not invalidated
        assert(vec.end() == initial_end);
    }

    //////////////////////////////////////
    // Testing move ctor and assignment //
    //////////////////////////////////////
    tester::set_limit(11); //5 * 2 + 1
    initial_begin = vec.begin();
    initial_end = vec.end();
    limited_vector<tester> move_ctor(std::move(vec));
    assert(move_ctor.begin() == initial_begin);
    assert(move_ctor.end() == initial_end);

    initial_begin = vec_b.begin();
    initial_end = vec_b.end();
    limited_vector<tester> move_assign;
    move_assign = std::move(vec_b);
    assert(move_assign.begin() == initial_begin);
    assert(move_assign.end() == initial_end);

    assert(tester::livecount == 11);

    move_ctor.pop_back();
    assert(move_ctor.size() == 4);
    assert(move_ctor.capacity() == 5);

    tester::set_limit(14); //4 * 2 + 5 + 1
    move_ctor.shrink_to_fit();
    assert(move_ctor.capacity() == move_ctor.size());

    //////////////////////////////////////////
    // Testing initializer list constructor //
    //////////////////////////////////////////
    tester::set_limit(14); //4 + 5 + 1 + 2
    limited_vector<tester> list_vec = { a, a };
    assert(list_vec.size() == 2);
    assert(list_vec.capacity() == 2);

    tester::set_limit(20);
    list_vec = { a, a, a};


    return;
}

void test_strings() {

    //////////////////////////////////////////////////////////////
    // Comparing performance of limited_vector with std::vector //
    // in pushing std::strings at the back                      //
    //////////////////////////////////////////////////////////////

    chrono::time_point<chrono::system_clock> start, end;

    cout << "Testing std::vector (with reserve called)" << endl;
    std::vector<std::string> vec;
    vec.reserve(1000000);
    std::string test_string("thequickbrownfoxjumpedoverthelazydog");
    cout << "Copying 1,000,000 elements" << endl;
    start = chrono::high_resolution_clock::now();
    try {
        for(int i = 0; i < 1000000; i++) 
            vec.push_back(test_string);
    }
    catch(...) {}
    end = chrono::high_resolution_clock::now(); 
    chrono::duration<double> elapsed_seconds = end - start; 
    cout << "elapsed time: " << chrono::duration_cast<chrono::milliseconds>(elapsed_seconds).count() << "ms\n";

    vec.clear();
    cout << "Moving 1,000,000 elements" << endl;
    start = chrono::high_resolution_clock::now();
    try {
        for(int i = 0; i < 1000000; i++) 
            vec.push_back(std::string(test_string));
    }
    catch(...) {}
    end = chrono::high_resolution_clock::now(); 
    elapsed_seconds = end - start; 
    cout << "elapsed time: " << chrono::duration_cast<chrono::milliseconds>(elapsed_seconds).count() << "ms\n";

    cout << endl << "Testing limited_vector" << endl;
    limited_vector<std::string> limvec(1000000);
    cout << "Copying 1,000,000 elements" << endl;
    start = chrono::high_resolution_clock::now();
    try {
        for(int i = 0; i < 1000000; i++) 
            limvec.push_back(test_string);
    }
    catch(...) {}
    end = chrono::high_resolution_clock::now(); 
    elapsed_seconds = end - start; 
    cout << "elapsed time: " << chrono::duration_cast<chrono::milliseconds>(elapsed_seconds).count() << "ms\n";

    limvec.clear();
    cout << "Moving 1,000,000 elements" << endl;
    start = chrono::high_resolution_clock::now();
    try {
        for(int i = 0; i < 1000000; i++) 
            limvec.push_back(std::string(test_string));
    }
    catch(...) {}
    end = chrono::high_resolution_clock::now(); 
    elapsed_seconds = end - start; 
    cout << "elapsed time: " << chrono::duration_cast<chrono::milliseconds>(elapsed_seconds).count() << "ms\n";

}

int main() {

    assert(tester::livecount == 0);
    test();
    assert(tester::livecount == 0); // both vectors destroyed

    test_strings();

    cout << endl << "All tests passed!" << endl;
    return 0;
}