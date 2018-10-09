from FifoPolicy import FifoPolicy


class SecChancePolicy(FifoPolicy):
    def put(self, frameId):
        """Allocates this frameId for some page"""
        self.fifo.append([False, frameId])

    def evict(self):
        """Deallocates a frame from the physical memory and returns its frameId"""
        item = self.fifo.pop(0)
        if (item[0]):
            return item[1]
        else:
             item[0] = not item[0]
             self.fifo.append(item)
        """verificar se o self.evict() eh o do FIfoPolicy ou o do SecChancePolicy"""
        return self.evict()
