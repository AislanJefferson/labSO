from FifoPolicy import FifoPolicy

class SecChancePolicy(FifoPolicy):
    def put(self, frameId):
        self.fifo.append([False, frameId])

    def evict(self):
        r,frame_id = self.fifo.pop(0)
        if (not r):
            return frame_id
        else:
             self.fifo.append([False,frame_id])
        return self.evict()

    def access(self, frameId, isWrite):
        for frame in self.fifo:
            if(frameId == frame[1]):
                frame[0] = True
