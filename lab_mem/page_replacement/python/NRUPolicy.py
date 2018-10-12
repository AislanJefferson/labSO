class NRUPolicy:
    def put(self, frameId):
        pass

    def evict(self):
        return 0

    def clock(self):
        pass

    def access(self, frameId, isWrite):
        pass
