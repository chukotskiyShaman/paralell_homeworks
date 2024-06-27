class Laplace {
private:
    double* A, * Anew;
    int n;

public:
    Laplace(int n);
    ~Laplace();
    void calcNext();
    double error_calc();
    void swap();
};