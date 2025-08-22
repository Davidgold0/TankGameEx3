namespace Algorithm_123456789 {

    class TankAlgorithm_123456789 : public TankAlgorithm {
    public:
        TankAlgorithm_123456789(int, int) {}
        ActionRequest getAction() override {
            return ActionRequest::Shoot;
        }
        void updateBattleInfo(BattleInfo&) override {}
    };
    
}