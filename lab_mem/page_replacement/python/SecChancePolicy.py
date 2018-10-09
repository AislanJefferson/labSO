from FifoPolicy import FifoPolicy


class SecChancePolicy(FifoPolicy):
    def put(self, frameId):
        """Allocates this frameId for some page"""
        self.fifo.append([False, frameId])

    def evict(self):
        """Deallocates a frame from the physical memory and returns its frameId"""
        item = self.fifo.pop(0)
        if (not item[0]):
            return item[1]
        else:
             self.fifo.append(item)
        """verificar se o self.evict() eh o do FIfoPolicy ou o do SecChancePolicy"""
        return self.evict()

    def access(self, frameId, isWrite):
        for frame in frames:
            if(frameId == frame[1]):
                frameId[0] = True
