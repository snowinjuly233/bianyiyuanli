void noop() {
    return;
}

int main() {
    while (noop()) {
        return 1;
    }
    return 0;
}
