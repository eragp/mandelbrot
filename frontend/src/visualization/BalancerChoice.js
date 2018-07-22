import React, { Component } from 'react';

export default class extends Component {

    componentWillMount() {
        this.setState({
            items: [{
                    key: 'naive',
                    title: "Naive Balancer",
                    active: true
                },
                {
                    key: 'column',
                    title: "Column Balancer"
                },
                {
                    key: 'prediction',
                    title: "Prediction Balancer",
                    disabled: true
                },
            ]
        });
    }

    handleBalancerChoice(balancer) {
        this.props.balancerPolicy.setBalancer(balancer);
        this.setState((oldState) => {
            for(let i in oldState.items){
                let item = oldState.items[i];
                if(item.key == balancer){
                    item.active = true;
                } else {
                    item.active = false;
                }
            }
            return oldState;
        });
    }

    render(){
        let itemList = [];
        for(let i in this.state.items){
            let item = this.state.items[i];
            let handler = item.disabled ? ()=>false : () => this.handleBalancerChoice(item.key);
            itemList.push(
                <BalancerItem title={item.title} active={item.active} disabled={item.disabled} handleClick={handler}/>
            )
        }

        return (
        <div class="list-group balancerChoice">
            {itemList}
        </div>);
    }

    
}

class BalancerItem extends Component {

    render() {
        let classList = "list-group-item list-group-item-action";
        if(this.props.active){
            classList += " active";
        }
        if(this.props.disabled){
            classList += " disabled";
        }
        return <a href="#" class={classList} onClick={this.props.handleClick} >{this.props.title}</a>;
    }

}