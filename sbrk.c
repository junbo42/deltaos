
extern int _sbrk(int incr);
extern char _end[];

int sbrk(int incr){
    static unsigned int heap_end;
    unsigned int prev_heap_end;
    
    if(heap_end == 0){
        heap_end = _sbrk((unsigned int)_end);
    }

    prev_heap_end = heap_end;
    
    if(incr != 0){
        heap_end = _sbrk(incr);
    }

    return prev_heap_end;
}   
