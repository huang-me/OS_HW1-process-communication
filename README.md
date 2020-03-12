# 2019_os_hw1
```
name: process communication  
Register the id and Send or Recv message from other users.  
```

### Compile
```
    cd module  
    make  
    make ins  
    cd ..  
    make  
```
    
### Run    
```
    ./com_app id queued/unqueued  
    id: The id you want to register.  
    queued: The message send to the id you register will be stored and will display in chronological order.  
    unqueued: Only store the newest message.  
```    
    
### Clean
```
    make clean  
    cd module  
    make rm  
    make clean  
```    
    
    
