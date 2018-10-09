from FifoPolicy import FifoPolicy


class SecChancePolicy(FifoPolicy):
    def put(self, frameId):
        """Allocates this frameId for some page"""
        self.fifo.append([False, frameId])

    def evict(self):
        """Deallocates a frame from the physical memory and returns its frameId"""
        leitura,item = self.fifo.pop(0)
        if (not leitura):
            return item
        else:
             self.fifo.append([0,item])
        """verificar se o self.evict() eh o do FIfoPolicy ou o do SecChancePolicy"""
        return self.evict()

    def access(self, frameId, isWrite):
        for frame in self.fifo:
            if(frameId == frame[1]):
                frameId[0] = True
